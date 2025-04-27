#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h> 
#include <linux/if_packet.h>
#include <net/if.h>
#include <signal.h>

#define BUFFER_SIZE 65536
#define DEFAULT_PORT 8159
#define DUMP_FILE "network_dump.bin"

int running = 1;
FILE *dump_file = NULL;

void sigint_handler(int sig) {
    running = 0;
    if (dump_file) fclose(dump_file);
    printf("\nЗавершение работы...\n");
    exit(EXIT_SUCCESS);
}

void process_packet(unsigned char *buffer, int size) {
    struct iphdr *ip_header = (struct iphdr*)buffer;

    if (ip_header->protocol != IPPROTO_UDP) return;

    struct sockaddr_in source, dest;
    memset(&source, 0, sizeof(source));
    memset(&dest, 0, sizeof(dest));
    
    source.sin_addr.s_addr = ip_header->saddr;
    dest.sin_addr.s_addr = ip_header->daddr;

    unsigned short iphdrlen = ip_header->ihl * 4;
    
    struct udphdr *udp_header = (struct udphdr*)(buffer + iphdrlen);
    if (ntohs(udp_header->dest) != DEFAULT_PORT) return;

    if (fwrite(buffer, 1, size, dump_file) < 0) {
        perror("fwrite");
        return;
    }
    if (fflush(dump_file) != 0){
        perror("fflush");
        return;
    }

    printf("Пакет от %s:%d -> %s:%d [%d байт]\n",
            inet_ntoa(source.sin_addr),
            ntohs(((struct udphdr*)(buffer + iphdrlen))->source),
            inet_ntoa(dest.sin_addr),
            DEFAULT_PORT,
            size);
}

int main() {
    int raw_socket;
    struct sockaddr saddr;
    socklen_t len;
    unsigned char buffer[BUFFER_SIZE];
    
    signal(SIGINT, sigint_handler);

    if ((raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    dump_file = fopen(DUMP_FILE, "wb");
    if (!dump_file) {
        perror("fopen");
        close(raw_socket);
        exit(EXIT_FAILURE);
    }

    printf("Начало захвата трафика на порт %d...\n", DEFAULT_PORT);

    while (running) {
        int data_size = recvfrom(raw_socket, buffer, BUFFER_SIZE, 0, &saddr, &len);
        if (data_size < 0) {
            perror("recvfrom");
            continue;
        }
        process_packet(buffer, data_size);
    }

    close(raw_socket);
    fclose(dump_file);
    return 0;
}