#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8159
#define BUFFER_SIZE 1024

int main() {
    char buffer[BUFFER_SIZE]; 
    char message[BUFFER_SIZE]; 
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr)); 
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Введите сообщение: ");

        if (!fgets(message, BUFFER_SIZE, stdin)) break;

        sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        
        if (strcmp(message, "exit\n") == 0) {
            break;
        }

        int len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL); 
        if (len < 0) {
            perror("recvfrom");
            continue;
        } else {
            buffer[len] = '\0'; 
            printf("Получено: %s\n", buffer); 
            if (strcmp(buffer, "exit\n") == 0)
                break;
        }  
    }

    close(sockfd);
    return 0;
}