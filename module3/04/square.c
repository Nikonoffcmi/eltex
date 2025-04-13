#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_count>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *endptr;
    long count = strtol(argv[1], &endptr, 10);
    if (endptr == argv[1] || *endptr != '\0' || count <= 0 || count > INT_MAX) {
        fprintf(stderr, "Invalid count\n");
        exit(EXIT_FAILURE);
    }
    int num_count = (int)count;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Дочерний процесс
        close(pipefd[0]); // Закрываем ненужный конец для чтения

        srand(time(NULL) ^ getpid());
        for (int i = 0; i < num_count; i++) {
            int num = rand() % 100;
            if (write(pipefd[1], &num, sizeof(num)) != sizeof(num)) {
                perror("write");
                close(pipefd[1]);
                exit(EXIT_FAILURE);
            }
        }

        close(pipefd[1]);
        exit(EXIT_SUCCESS);
    } else { // Родительский процесс
        close(pipefd[1]); // Закрываем ненужный конец для записи

        int output_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd == -1) {
            perror("open");
            close(pipefd[0]);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < num_count; i++) {
            int num;
            ssize_t bytes_read;
            char *buffer = (char *)&num;
            size_t bytes_remaining = sizeof(num);
            while (bytes_remaining > 0) {
                bytes_read = read(pipefd[0], buffer, bytes_remaining);
                if (bytes_read == -1) {
                    perror("read");
                    close(pipefd[0]);
                    close(output_fd);
                    exit(EXIT_FAILURE);
                } else if (bytes_read == 0) {
                    fprintf(stderr, "Unexpected end of data\n");
                    close(pipefd[0]);
                    close(output_fd);
                    exit(EXIT_FAILURE);
                }
                bytes_remaining -= bytes_read;
                buffer += bytes_read;
            }

            char output_buffer[32];
            int len = snprintf(output_buffer, sizeof(output_buffer), "%d\n", num);
            if (write(STDOUT_FILENO, output_buffer, len) != len) {
                perror("write stdout");
                close(pipefd[0]);
                close(output_fd);
                exit(EXIT_FAILURE);
            }
            if (write(output_fd, output_buffer, len) != len) {
                perror("write file");
                close(pipefd[0]);
                close(output_fd);
                exit(EXIT_FAILURE);
            }
        }

        close(pipefd[0]);
        close(output_fd);

        wait(NULL);
    }

    return EXIT_SUCCESS;
}