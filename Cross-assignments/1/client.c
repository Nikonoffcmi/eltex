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
#define SERVER_TIMEOUT 2

volatile sig_atomic_t flag = 0;

void handle_signal(int sig) {
    flag = 1;
}

unsigned short csum(unsigned short *buf, int nwords) {
    unsigned long sum = 0;
    while (nwords--) sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port> <client_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct in_addr server_ip;
    if (inet_aton(argv[1], &server_ip) == 0) {
        fprintf(stderr, "Invalid server IP address\n");
        exit(EXIT_FAILURE);
    }
    
    int server_port = atoi(argv[2]);
    int client_port = atoi(argv[3]);
    if (server_port <= 1024 || server_port > 65535 || client_port <= 1024 || client_port > 65535) {
        fprintf(stderr, "Invalid port numbers\n");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; 

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
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
        exit(EXIT_FAILURE);
    }

    struct timeval tv = {.tv_sec = SERVER_TIMEOUT, .tv_usec = 0};
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt timeout");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char packet[BUFF_SIZE];
    struct iphdr *ip_header = (struct iphdr*)packet;
    struct udphdr *udp_header = (struct udphdr*)(packet + sizeof(struct iphdr));
    char *data = (char*)udp_header + sizeof(struct udphdr);

    ip_header->version = 4;
    ip_header->ihl = 5;
    ip_header->tos = 0;
    ip_header->id = htons(0);
    ip_header->frag_off = 0;
    ip_header->ttl = 64;
    ip_header->protocol = IPPROTO_UDP;
    ip_header->daddr = server_ip.s_addr;

    udp_header->source = htons(client_port);
    udp_header->dest = htons(server_port);

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr = server_ip,
        .sin_port = htons(server_port)
    };

    while (!flag) {
        printf("Enter message: ");
        
        if (!fgets(data, BUFF_SIZE/2, stdin)) {
            if (errno == EINTR && flag) 
                break;
            else if (ferror(stdin)) {
                perror("fgets");
                clearerr(stdin);
            }
            continue;
        }

        size_t data_len = strlen(data);
        if (data_len > 0 && data[data_len-1] == '\n') {
            data[--data_len] = '\0';
        } else {
            fprintf(stderr, "Input too long or invalid\n");
            while (getchar() != '\n');
            continue;
        }

        if (strcmp(data, "CLOSE") == 0) {
            printf("Exit...");
            break;
        }

        if (data_len == 0) {
            fprintf(stderr, "Empty message\n");
            continue;
        }

        udp_header->len = htons(sizeof(struct udphdr) + data_len);
        ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + data_len);
        ip_header->check = 0;
        ip_header->check = csum((unsigned short*)ip_header, sizeof(struct iphdr)/2);

        struct pseudo_header {
            u_int32_t src, dst;
            u_int8_t zero;
            u_int8_t proto;
            u_int16_t len;
        } pseudo = {
            .src = ip_header->saddr,
            .dst = ip_header->daddr,
            .zero = 0,
            .proto = IPPROTO_UDP,
            .len = udp_header->len
        };

        char tmp[sizeof(pseudo) + sizeof(struct udphdr) + data_len];
        memcpy(tmp, &pseudo, sizeof(pseudo));
        memcpy(tmp + sizeof(pseudo), udp_header, sizeof(struct udphdr) + data_len);
        udp_header->check = 0;
        udp_header->check = csum((unsigned short*)tmp, sizeof(tmp)/2);
        
        ssize_t sent = sendto(sockfd, packet, ntohs(ip_header->tot_len), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if (sent < 0) {
            perror("sendto");
            continue;
        }

        char recv_buf[BUFF_SIZE];
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);
        
        while (1) {

            ssize_t recv_len = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr*)&from_addr, &from_len);
            if (recv_len < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    fprintf(stderr, "Server response timeout\n");
                } else {
                    perror("recvfrom");
                }
                break;
            }

            if (from_addr.sin_addr.s_addr != server_ip.s_addr) {
                continue; 
            }

            struct iphdr *recv_ip = (struct iphdr*)recv_buf;
            if (recv_ip->protocol != IPPROTO_UDP || recv_len < (ssize_t)(recv_ip->ihl*4 + sizeof(struct udphdr))) {
                continue;
            }

            struct udphdr *recv_udp = (struct udphdr*)(recv_buf + recv_ip->ihl*4);

            if (ntohs(recv_udp->dest) != client_port) continue;
            if (ntohs(recv_udp->source) != server_port) continue;

            char *recv_data = (char*)recv_udp + sizeof(struct udphdr);
            int recv_data_len = ntohs(recv_udp->len) - sizeof(struct udphdr);
            
            if (recv_data_len <= 0 || recv_data_len > (recv_len - recv_ip->ihl*4 - sizeof(struct udphdr))) {
                fprintf(stderr, "Invalid response format\n");
                break;
            }

            printf("Response: %.*s\n", recv_data_len, recv_data);
            break;
        }
    }

    strcpy(data, "CLOSE");
    size_t data_len = strlen(data);

    udp_header->len = htons(sizeof(struct udphdr) + data_len);
    ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + data_len);
    ip_header->check = 0;
    ip_header->check = csum((unsigned short*)ip_header, sizeof(struct iphdr)/2);

    struct pseudo_header {
        u_int32_t src, dst;
        u_int8_t zero;
        u_int8_t proto;
        u_int16_t len;
    } pseudo_close = {
        .src   = ip_header->saddr,
        .dst   = ip_header->daddr,
        .zero  = 0,
        .proto = IPPROTO_UDP,
        .len   = udp_header->len
    };

    char tmp_close[sizeof(pseudo_close) + sizeof(struct udphdr) + data_len];
    memcpy(tmp_close, &pseudo_close, sizeof(pseudo_close));
    memcpy(tmp_close + sizeof(pseudo_close), udp_header, sizeof(struct udphdr) + data_len);
    udp_header->check = 0;
    udp_header->check = csum((unsigned short*)tmp_close, sizeof(tmp_close)/2);

    ssize_t sent = sendto(sockfd, packet, ntohs(ip_header->tot_len), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (sent < 0) {
        perror("Final sendto failed");
    }

    close(sockfd);
    return EXIT_SUCCESS;
}