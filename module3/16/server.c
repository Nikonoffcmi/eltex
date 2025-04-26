#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define FILENAME_SIZE 256

void dostuff(int);
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
        if (pid < 0) {
            close(newsockfd);
            error("ERROR on fork");
        }
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
    char buff[BUFFER_SIZE];
    int bytes_recv;
    
    const char *op_prompt = "Enter operation (+, -, *, /, file):\r\n";
    
    if (write(sock, op_prompt, strlen(op_prompt)) < 0) 
            error("ERROR writing operation");

    if ((bytes_recv = recv(sock, buff, sizeof(buff), 0)) <= 0) {
        perror("ERROR receiving operation");
        close(sock);
        return;
    }
    buff[bytes_recv] = '\0';


    if (strncmp(buff, "FILE", 4) == 0) {
        char filename[BUFFER_SIZE];
        long file_size;

        if (write(sock, "ok", strlen("ok")) < 0) 
            error("ERROR writing operation");
        
        if ((bytes_recv = recv(sock, filename, sizeof(filename), 0)) <= 0) {
            perror("ERROR receiving filename");
            close(sock);
            return;
        }
        filename[bytes_recv] = '\0';

        if (write(sock, "ok", strlen("ok")) < 0) 
            error("ERROR writing operation");
        
        
        if (recv(sock, &file_size, sizeof(file_size), 0) != sizeof(file_size)) {
            perror("ERROR receiving file size");
            close(sock);
            return;
        }

        if (write(sock, "ok", strlen("ok")) < 0) 
            error("ERROR writing operation");
        

        printf("Receiving file: %s (%ld bytes)\n", filename, file_size);
        receive_file(sock, filename, file_size);
    } else {
        int a, b, result;
        char operation = buff[0], message[BUFFER_SIZE];;

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
    close(sock);

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
        write(fd, buffer, bytes);
        total += bytes;
    }
    
    close(fd);
    printf("File received: %s (%ld bytes)\n", filename, total);
}