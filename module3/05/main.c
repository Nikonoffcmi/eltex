#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include <signal.h>

volatile int file_locked = 0;

void sigusr1_handler(int sig) {
    file_locked = 1;
}

void sigusr2_handler(int sig) {
    file_locked = 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <количество_чисел>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *endptr;
    long count = strtol(argv[1], &endptr, 10);
    if (endptr == argv[1] || *endptr != '\0' || count <= 0 || count > INT_MAX) {
        fprintf(stderr, "Неверное количество\n");
        exit(EXIT_FAILURE);
    }
    int num_count = (int)count;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    FILE *init_file = fopen("input.txt", "w");
    if (init_file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    fprintf(init_file, "0\n");
    fclose(init_file);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        signal(SIGUSR1, sigusr1_handler); 
        signal(SIGUSR2, sigusr2_handler);   

        close(pipefd[0]);
        srand(time(NULL));

        for (int i = 0; i < num_count; i++) {
            while (file_locked) pause();

            FILE *file = fopen("input.txt", "r");
            if (file == NULL) {
                perror("Failed to open file");
                exit(EXIT_FAILURE);
            }
            int num;
            fscanf(file, "%d", &num);
            fclose(file);
            

            num = rand() % 1000;
            if (write(pipefd[1], &num, sizeof(num)) != sizeof(num)) {
                perror("write");
                close(pipefd[1]);
                exit(EXIT_FAILURE);
            }
        }

        close(pipefd[1]);
        exit(EXIT_SUCCESS);
    } else { 
        signal(SIGUSR1, SIG_IGN);
        signal(SIGUSR2, SIG_IGN);

        close(pipefd[1]);
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
                    fprintf(stderr, "Неожиданный конец передачи данных\n");
                    close(pipefd[0]);
                    close(output_fd);
                    exit(EXIT_FAILURE);
                }
                bytes_remaining -= bytes_read;
                buffer += bytes_read;
            }

            char buf[32];
            int len = snprintf(buf, sizeof(buf), "%d\n", num);
            if (write(STDOUT_FILENO, buf, len) != len) {
                perror("write stdout");
                close(pipefd[0]);
                close(output_fd);
                exit(EXIT_FAILURE);
            }
            if (write(output_fd, buf, len) != len) {
                perror("write file");
                close(pipefd[0]);
                close(output_fd);
                exit(EXIT_FAILURE);
            }

            kill(pid, SIGUSR1);
            FILE *file = fopen("input.txt", "a");
            if (file == NULL) {
                perror("Failed to open file");
                exit(EXIT_FAILURE);
            }
            fprintf(file, "%d\n", num);
            fclose(file);
            kill(pid, SIGUSR2);
        }

        close(pipefd[0]);
        close(output_fd);
        int status;
        wait(&status);
        if (!WIFEXITED(status))
        {
            printf("child not exited");
            exit(0);
        }
    }

    return EXIT_SUCCESS;
}