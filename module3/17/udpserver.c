#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8159
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client1_addr, client2_addr;
    char buffer[BUFFER_SIZE];
    int client1_connected = 0, client2_connected = 0;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    bzero(&server_addr, sizeof(server_addr)); 
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Сервер запущен на порту %d\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        ssize_t recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
        if (recv_len < 0) {
            perror("recvfrom");
            continue;
        }
        buffer[recv_len] = '\0';

        int is_client1 = 0, is_client2 = 0;
        if (client1_connected) {
            if (client_addr.sin_port == client1_addr.sin_port && 
                client_addr.sin_addr.s_addr == client1_addr.sin_addr.s_addr) {
                is_client1 = 1;
            }
        }
        if (client2_connected) {
            if (client_addr.sin_port == client2_addr.sin_port && 
                client_addr.sin_addr.s_addr == client2_addr.sin_addr.s_addr) {
                is_client2 = 1;
            }
        }

        if (!is_client1 && !is_client2) {
            if (!client1_connected) {
                client1_addr = client_addr;
                client1_connected = 1;
                printf("Клиент 1 подключен: %s:%d\n", 
                        inet_ntoa(client1_addr.sin_addr), ntohs(client1_addr.sin_port));
            } else if (!client2_connected) {
                client2_addr = client_addr;
                client2_connected = 1;
                printf("Клиент 2 подключен: %s:%d\n", 
                        inet_ntoa(client2_addr.sin_addr), ntohs(client2_addr.sin_port));
            } else {
                printf("Достигнут лимит клиентов.\n");
                continue;
            }
        }

        if (client1_connected && client2_connected) {
            struct sockaddr_in *target = is_client1 ? &client2_addr : &client1_addr;
            if (sendto(sockfd, buffer, recv_len, 0, (struct sockaddr*)target, sizeof(*target)) < 0) {
                perror("sendto");
            }
        }
    }

    close(sockfd);
    return 0;
}