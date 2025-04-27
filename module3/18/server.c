#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>


#define BUFFER_SIZE 1024
#define FILENAME_SIZE 256
#define MAX_CLIENTS 1024

void dostuff(int, int*);
void receive_file(int sock, const char *filename, long file_size);

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int nclients = 0;

void printusers() { 
    if(nclients)
        printf("%d user on-line\n",nclients);
    else 
        printf("No User on line\n");
}

int main(int argc, char *argv[]) {
    printf("TCP SERVER\n");
    int sockfd, newsockfd;
    int portno; 
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    fd_set readfds, temp_fds;
    int client_sockets[MAX_CLIENTS];

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(EXIT_FAILURE);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
    
    listen(sockfd,5);
    clilen = sizeof(cli_addr);

    for (int i = 0; i < MAX_CLIENTS; i++)
				client_sockets[i] = -1;

    FD_ZERO(&readfds);
    FD_ZERO(&temp_fds);
    FD_SET(sockfd, &readfds);
    int fd_max = sockfd;

	while (1) {
        temp_fds = readfds;

        if (select(fd_max + 1, &temp_fds, NULL, NULL, NULL) < 0)
            error("ERROR on select");
        
        for (int i = 0; i <= fd_max; i++) 
        {
            if (FD_ISSET(i, &temp_fds)) 
            {
                if (i == sockfd) 
                {
                    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
                    if (newsockfd < 0) {
                        perror("ERROR on accept");
                        continue;
                    }
                        
                    int j;
                    for (j = 0; j < MAX_CLIENTS; j++) 
                    {
                        if (client_sockets[j] == -1) {
                            client_sockets[j] = newsockfd;
                            nclients++;
                            break;
                        }
                    }
                    if (j == MAX_CLIENTS) {
                        fprintf(stderr, "Too many clients\n");
                        close(newsockfd);
                        continue;
                    }    

                    FD_SET(newsockfd, &readfds);
                    if (newsockfd > fd_max) 
                        fd_max = newsockfd;
                        
                    const char *op_prompt = "Enter operation (+, -, *, /, file):\r\n";
                    if (write(newsockfd, op_prompt, strlen(op_prompt)) < 0) {
                        perror("ERROR writing operation prompt");
                        close(newsockfd);
                        FD_CLR(newsockfd, &readfds);
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (client_sockets[j] == newsockfd) {
                                client_sockets[j] = -1;
                                nclients--;
                                break;
                            }
                        }
                    }

                    printusers();
                } else {
                    int close_conn = 0;
                    dostuff(i, &close_conn);
                    if (close_conn) {
                        close(i);
                        FD_CLR(i, &readfds);
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (client_sockets[j] == i) {
                                client_sockets[j] = -1;
                                nclients--;
                                break;
                            }
                        }
                        if (i == fd_max) {
                            fd_max = sockfd;
                            for (int j = 0; j < MAX_CLIENTS; j++) {
                                if (client_sockets[j] != -1 && client_sockets[j] > fd_max) {
                                    fd_max = client_sockets[j];
                                }
                            }
                        }
                    }
                }
                
            }
        }
	}
	close(sockfd);
	return 0;
}

void dostuff(int sock, int* close_conn) {
    char buff[BUFFER_SIZE];
    int bytes_recv;

    if ((bytes_recv = recv(sock, buff, sizeof(buff), 0)) <= 0) {
        perror("ERROR receiving operation");
        *close_conn = 1;
        return;
    }
    buff[bytes_recv] = '\0';


    if (strncmp(buff, "FILE", 4) == 0) {
        char filename[BUFFER_SIZE];
        long file_size;

        if (write(sock, "ok", strlen("ok")) < 0) {
            perror("ERROR writing ok");
            *close_conn = 1;
            return;
        }
        
        if ((bytes_recv = recv(sock, filename, sizeof(filename), 0)) <= 0) {
            perror("ERROR receiving filename");
            *close_conn = 1;
            return;
        }
        filename[bytes_recv] = '\0';

        if (write(sock, "ok", strlen("ok")) < 0) {
            perror("ERROR writing ok");
            *close_conn = 1;
            return;
        }
        
        if (recv(sock, &file_size, sizeof(file_size), 0) != sizeof(file_size)) {
            perror("ERROR receiving file size");
            *close_conn = 1;
            return;
        }

        if (write(sock, "ok", strlen("ok")) < 0) {
            perror("ERROR writing ok");
            *close_conn = 1;
            return;
        }
        

        printf("Receiving file: %s (%ld bytes)\n", filename, file_size);
        receive_file(sock, filename, file_size);
    } else {
        int a, b, result;
        char operation = buff[0], message[BUFFER_SIZE];;

        const char *num1_prompt = "Enter first parameter:\r\n";
        
        if (write(sock, num1_prompt, strlen(num1_prompt)) < 0) {
            perror("ERROR writing num1 prompt");
            *close_conn = 1;
            return;
        }

        bytes_recv = read(sock, buff, sizeof(buff)-1);
        if (bytes_recv < 0) {
            error("ERROR reading first parameter");
            *close_conn = 1;
            return;
        }
        
        buff[bytes_recv] = '\0';
        a = atoi(buff);

        const char *num2_prompt = "Enter second parameter:\r\n";
        
        if (write(sock, num2_prompt, strlen(num2_prompt)) < 0)  {
            perror("ERROR writing num2 prompt");
            *close_conn = 1;
            return;
        }

        bytes_recv = read(sock, buff, sizeof(buff)-1);
        if (bytes_recv < 0) {
            error("ERROR reading second parameter");
            *close_conn = 1;
            return;
        }
        
        buff[bytes_recv] = '\0';
        b = atoi(buff);

        if (operation == '+') 
            result = a + b;
        else if (operation == '-') 
            result = a - b;
        else if (operation == '*') 
            result = a * b;
        else if (operation == '/') {
            if (b == 0) {
                strcpy(message, "Error: division by zero\r\n");
                if (write(sock, message, strlen(message)) < 0) 
                    perror("ERROR writing operation");
                *close_conn = 1;
                return;
            } else 
                result = a / b;
        } else {
            strcpy(message, "Error: invalid operation\r\n");
            if (write(sock, message, strlen(message)) < 0) 
                perror("ERROR writing operation");
            *close_conn = 1;
            return;
        }
        snprintf(message, sizeof(message), "Result: %d\r\n", result);
        if (write(sock, message, strlen(message)) < 0) 
            perror("ERROR writing operation");
        *close_conn = 1;
        return;
    }
    *close_conn = 1;
}

void receive_file(int sock, const char *filename, long file_size) {
    int fd = open("recv.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        perror("File create error");
        return;
    }
    
    char buffer[BUFFER_SIZE];
    long total = 0;
    
    while (total < file_size) {
        int bytes = recv(sock, buffer, BUFFER_SIZE-1, 0);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';
        if (write(fd, buffer, bytes) < 0) {
            perror("write file error");
            return;
        }
        total += bytes;
    }
    
    close(fd);
    printf("File received: %s (%ld bytes)\n", filename, total);
}