#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <errno.h>

#define BUFF_SIZE 2048

volatile sig_atomic_t shutdown_flag = 0;

void handle_signal(int sig) {
    shutdown_flag = 1;
}

struct client_node {
    struct in_addr ip;
    u_short port;
    int counter;
    struct client_node *next;
};

struct client_node *clients = NULL;

struct client_node *find_client(struct in_addr ip, u_short port) {
    struct client_node *current = clients;
    while (current) {
        if (current->ip.s_addr == ip.s_addr && current->port == port)
            return current;
        current = current->next;
    }
    return NULL;
}

void add_client(struct in_addr ip, u_short port) {
    struct client_node *new_node = malloc(sizeof(struct client_node));
    if (!new_node) {
        perror("malloc failed");
        return;
    }
    new_node->ip = ip;
    new_node->port = port;
    new_node->counter = 1;
    new_node->next = clients;
    clients = new_node;
}

void remove_client(struct in_addr ip, u_short port) {
    struct client_node **current = &clients;
    while (*current) {
        if ((*current)->ip.s_addr == ip.s_addr && (*current)->port == port) {
            struct client_node *temp = *current;
            *current = temp->next;
            free(temp);
            return;
        }
        current = &(*current)->next;
    }
}

unsigned short csum(unsigned short *buf, int nwords) {
    unsigned long sum = 0;
    while (nwords--) sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; 

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    int server_port = atoi(argv[1]);
    if (server_port <= 1024 || server_port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int on = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);;
    }

    char buffer[BUFF_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while (!shutdown_flag) {
        ssize_t len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &addr_len);
        if (len < 0) {
            if (errno == EINTR) break;
            perror("recvfrom");
            continue;
        }

        if (len < sizeof(struct iphdr) + sizeof(struct udphdr)) {
            fprintf(stderr, "Invalid packet size\n");
            continue;
        }

        struct iphdr *ip_header = (struct iphdr*)buffer;
        if (ip_header->protocol != IPPROTO_UDP )
            continue;

        struct udphdr *udp_header = (struct udphdr*)(buffer + ip_header->ihl*4);
        if (ntohs(udp_header->dest) != server_port)
            continue;

        char *data = (char*)udp_header + sizeof(struct udphdr);
        int data_len = ntohs(udp_header->len) - sizeof(struct udphdr);
        if (data_len < 0 || data_len > (len - sizeof(struct iphdr) - sizeof(struct udphdr))) {
            fprintf(stderr, "Invalid data length\n");
            continue;
        }
        data[data_len] = '\0';

        struct in_addr client_ip = {ip_header->saddr};
        u_short client_port = ntohs(udp_header->source);

        if (strcmp(data, "CLOSE") == 0) {
            remove_client(client_ip, client_port);
            printf("Client %s:%d disconnected\n", inet_ntoa(client_ip), client_port);
            continue;
        }

        struct client_node *client = find_client(client_ip, client_port);
        if (!client) {
            add_client(client_ip, client_port);
            client = clients;
        } else {
            client->counter++;
        }

        char reply[BUFF_SIZE];
        int reply_len = snprintf(reply, sizeof(reply), "%s %d", data, client->counter);
        if (reply_len >= sizeof(reply)) {
            fprintf(stderr, "Reply too long\n");
            continue;
        }

        char packet[BUFF_SIZE];
        struct iphdr *reply_ip = (struct iphdr*)packet;
        struct udphdr *reply_udp = (struct udphdr*)(packet + sizeof(struct iphdr));
        char *reply_data = (char*)reply_udp + sizeof(struct udphdr);

        reply_ip->version = 4;
        reply_ip->ihl = 5;
        reply_ip->tos = 0;
        reply_ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + reply_len);
        reply_ip->id = htons(0);
        reply_ip->frag_off = 0;
        reply_ip->ttl = 64;
        reply_ip->protocol = IPPROTO_UDP;
        reply_ip->saddr = ip_header->daddr;
        reply_ip->daddr = ip_header->saddr;
        reply_ip->check = 0;
        reply_ip->check = csum((unsigned short*)reply_ip, sizeof(struct iphdr)/2);

        reply_udp->source = htons(server_port);
        reply_udp->dest = udp_header->source;
        reply_udp->len = htons(sizeof(struct udphdr) + reply_len);
        reply_udp->check = 0;

        memcpy(reply_data, reply, reply_len);

        struct pseudo_header {
            u_int32_t src, dst;
            u_int8_t zero;
            u_int8_t proto;
            u_int16_t len;
        } pseudo = {
            .src = reply_ip->saddr,
            .dst = reply_ip->daddr,
            .zero = 0,
            .proto = IPPROTO_UDP,
            .len = htons(sizeof(struct udphdr) + reply_len)
        };

        char tmp[sizeof(pseudo) + sizeof(struct udphdr) + reply_len];
        memcpy(tmp, &pseudo, sizeof(pseudo));
        memcpy(tmp + sizeof(pseudo), reply_udp, sizeof(struct udphdr) + reply_len);
        reply_udp->check = csum((unsigned short*)tmp, sizeof(tmp)/2);

        struct sockaddr_in dest_addr = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = reply_ip->daddr,
            .sin_port = reply_udp->dest
        };

        ssize_t sent = sendto(sockfd, packet, ntohs(reply_ip->tot_len), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
        if (sent < 0) {
            perror("sendto failed");
        } else if (sent != ntohs(reply_ip->tot_len)) {
            fprintf(stderr, "Partial send: %zd/%d bytes\n", sent, ntohs(reply_ip->tot_len));
        }
    }

    close(sockfd);
    while (clients) {
        struct client_node *temp = clients;
        clients = clients->next;
        free(temp);
    }

    return EXIT_SUCCESS;
}