#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8159
#define BUFFER_SIZE 1024

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT)
    };
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }


    fd_set read_fds;
    printf("Введите сообщение: ");
    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        
        if (select(sockfd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char msg[BUFFER_SIZE];
            if (!fgets(msg, BUFFER_SIZE, stdin)) break;
            sendto(sockfd, msg, strlen(msg), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        }

        if (FD_ISSET(sockfd, &read_fds)) {
            char buf[BUFFER_SIZE];
            ssize_t len = recvfrom(sockfd, buf, BUFFER_SIZE, 0, NULL, NULL);
            if (len < 0) {
                perror("recvfrom");
                continue;
            }
            buf[len] = '\0';
            printf("Получено: %s", buf);
        }
    }

    close(sockfd);
    return 0;
}