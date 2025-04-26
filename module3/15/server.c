#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

void dostuff(int);

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
    int pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

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
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");
        nclients++;
        printusers();
        pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0) {
            close(sockfd);
            dostuff(newsockfd);
            exit(EXIT_SUCCESS);
        } else 
            close(newsockfd);
    }
    close(sockfd);
    return 0;
}

void dostuff(int sock) {
    int bytes_recv, a, b, result;
    char op_buff[10], buff[BUFFER_SIZE], message[BUFFER_SIZE];

    while (1){
        const char *op_prompt = "Enter operation (+, -, *, /):\r\n";
        
        if (write(sock, op_prompt, strlen(op_prompt)) < 0) 
            error("ERROR writing operation");

        bytes_recv = read(sock, op_buff, sizeof(op_buff)-1);
        if (bytes_recv < 0) error("ERROR reading operation");
        
        op_buff[bytes_recv] = '\0';
        char operation = op_buff[0];

        const char *num1_prompt = "Enter first parameter:\r\n";
        
        if (write(sock, num1_prompt, strlen(num1_prompt)) < 0) 
            error("ERROR writing operation");

        bytes_recv = read(sock, buff, sizeof(buff)-1);
        if (bytes_recv < 0) error("ERROR reading first parameter");
        
        buff[bytes_recv] = '\0';
        a = atoi(buff);

        const char *num2_prompt = "Enter second parameter:\r\n";
        
        if (write(sock, num2_prompt, strlen(num2_prompt)) < 0) 
            error("ERROR writing operation");

        bytes_recv = read(sock, buff, sizeof(buff)-1);
        if (bytes_recv < 0) error("ERROR reading second parameter");
        
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
                    error("ERROR writing operation");
            } else 
                result = a / b;
        } else {
            strcpy(message, "Error: invalid operation\r\n");
            if (write(sock, message, strlen(message)) < 0) 
                error("ERROR writing operation");
        }
        snprintf(message, sizeof(message), "Result: %d\r\n", result);
        if (write(sock, message, strlen(message)) < 0) 
                error("ERROR writing operation");
    }
}