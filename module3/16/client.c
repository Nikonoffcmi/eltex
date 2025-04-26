#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>

#define BUFFER_SIZE 1024
#define FILENAME_SIZE 256

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void send_file(int sock, const char *filename) {
    int n;
    char buff[BUFFER_SIZE];
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        printf("File not found\n");
        return;
    }
    
    // Получаем размер файла
    long file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    
    
    // Отправляем заголовок
    send(sock, "FILE", 4, 0);
    n = recv(sock, buff, sizeof(buff)-1, 0);
    
    // Отправляем имя файла и размер
    send(sock, filename, strlen(filename), 0);
    n = recv(sock, buff, sizeof(buff)-1, 0);

    send(sock, &file_size, sizeof(file_size), 0);
    n = recv(sock, buff, sizeof(buff)-1, 0);

    
    // Отправляем содержимое файла
    char buffer[BUFFER_SIZE];
    long total = 0;
    
    while (total < file_size) {
        int bytes = read(fd, buffer, BUFFER_SIZE);
        if (bytes <= 0) break;
        send(sock, buffer, bytes, 0);
        total += bytes;
    }
    
    close(fd);
    printf("File sent: %s (%ld bytes)\n", filename, file_size);
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

        if (!strcmp(buff, "file\n")) 
		{
			char filename[FILENAME_SIZE];
            printf("Enter file path: ");
            if (scanf("%255s", filename) != 1) {
                printf("ERROR reading filename\n");
                while (getchar() != '\n');
                continue;
            }
            while (getchar() != '\n');
            
            send_file(my_sock, filename);
            close(my_sock);
            return 0;
		}

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