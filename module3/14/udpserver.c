#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8159
#define BUFFER_SIZE 1024

int main() {
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE]; 
    int sockfd;
    struct sockaddr_in server_addr;

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

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        ssize_t recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
        if (recv_len < 0) {
            perror("recvfrom");
            continue;
        }
        buffer[recv_len] = '\0';
        printf("Получено: %s\n", buffer); 
        if (strcmp(buffer, "exit\n") == 0)
            break;

        
        printf("Введите сообщение: ");
        if (!fgets(message, BUFFER_SIZE, stdin)) break;

        sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
        
        if (strcmp(message, "exit\n") == 0) {
            break;
        }
    }

    close(sockfd);
    return 0;
}