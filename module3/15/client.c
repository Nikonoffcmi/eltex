#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFFER_SIZE 1024

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int my_sock, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buff[BUFFER_SIZE];

    printf("TCP CLIENT\n");
    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    portno = atoi(argv[2]);
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(EXIT_SUCCESS);
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(my_sock,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    while ((n = recv(my_sock, buff, sizeof(buff)-1, 0)) > 0) {
        buff[n] = 0;
        printf("S=>C:%s", buff);
        printf("S<=C:");
        fgets(buff, sizeof(buff)-1, stdin);
        if (!strcmp(buff, "quit\n")) {
            printf("Exit...");
            close(my_sock);
            return 0;
        }
        
        if (send(my_sock, buff, strlen(buff), 0) < 0) 
                    error("ERROR send operation");
    }
    printf("Recv error \n");
    close(my_sock);
    return -1;
}