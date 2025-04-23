#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <errno.h>

#define SEM_NAME "/sem"
#define FILE_NAME "input.txt"

void cleanup(sem_t *sem) {
    sem_close(sem);
    sem_unlink(SEM_NAME);
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

    
    sem_t *sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 1);
    if (sem == SEM_FAILED) {
        if (errno == EEXIST) {
            sem_unlink(SEM_NAME);
            sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
        }
        if (sem == SEM_FAILED) {
            perror("sem_open");
            exit(EXIT_FAILURE);
        }
    }

    FILE *file = fopen(FILE_NAME, "w");
    if (file == NULL) {
        cleanup(sem);
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    fclose(file);

    int pipefd[2];
    if (pipe(pipefd)) {
        cleanup(sem);
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        cleanup(sem);
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        close(pipefd[0]);
        srand(time(NULL));

        for (int i = 0; i < num_count; i++) {
            if (sem_wait(sem) == -1) {
                perror("sem_wait child");
                sem_close(sem);
                close(pipefd[1]);
                exit(EXIT_FAILURE);
            }
            

            FILE *file = fopen(FILE_NAME, "r");
            if (file == NULL) {
                perror("Failed to open file");
                sem_close(sem);
                close(pipefd[1]);
                exit(EXIT_FAILURE);
            }
            int num;
            fscanf(file, "%d", &num);
            fclose(file);
            
            if (sem_post(sem) == -1) {
                perror("sem_post child");
                sem_close(sem);
                close(pipefd[1]);
                exit(EXIT_FAILURE);
            }

            num = rand() % 100;
            if (write(pipefd[1], &num, sizeof(num)) != sizeof(num)) {
                perror("write");
                close(pipefd[1]);
                sem_close(sem);
                exit(EXIT_FAILURE);
            }
            sleep(1);
        }

        close(pipefd[1]);
        sem_post(sem);
        exit(EXIT_SUCCESS);
    } else {
        close(pipefd[1]);
        int output_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd == -1) {
            perror("open");
            close(pipefd[0]);
            cleanup(sem);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < num_count; i++) {
            int num;
            if (read(pipefd[0], &num, sizeof(num)) == -1) {
                perror("read failed");
                close(output_fd);
                close(pipefd[0]);
                cleanup(sem);
                exit(EXIT_FAILURE);
            }

            if (sem_wait(sem) == -1) {
                perror("sem_wait p");
                close(output_fd);
                close(pipefd[0]);
                cleanup(sem);
                exit(EXIT_FAILURE);
            }

            FILE *file = fopen(FILE_NAME, "a");
            if (file == NULL) {
                perror("Failed to open file");
                close(output_fd);
                close(pipefd[0]);
                cleanup(sem);
                exit(EXIT_FAILURE);
            }
            fprintf(file, "%d\n", num);
            fclose(file);

            if (sem_post(sem) == -1) {
                perror("sem_post p");
                close(output_fd);
                close(pipefd[0]);
                cleanup(sem);
                exit(EXIT_FAILURE);
            }

            char buf[32];
            int len = snprintf(buf, sizeof(buf), "%d\n", num);
            if (write(STDOUT_FILENO, buf, len) != len) {
                perror("write stdout");
                close(pipefd[0]);
                close(output_fd);
                cleanup(sem);
                exit(EXIT_FAILURE);
            }
            if (write(output_fd, buf, len) != len) {
                perror("write file");
                close(pipefd[0]);
                close(output_fd);
                cleanup(sem);
                exit(EXIT_FAILURE);
            }

        }

        close(pipefd[0]);
        close(output_fd);
        wait(NULL);
        cleanup(sem);
    }

    return EXIT_SUCCESS;
}