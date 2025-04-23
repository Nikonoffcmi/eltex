#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

#define FILE_NAME "input.txt"

void sem_lock(int sem_id) {
    struct sembuf sops = {0, -1, 0};
    if (semop(sem_id, &sops, 1) == -1) {
        perror("semop lock");
        exit(EXIT_FAILURE);
    }
}

void sem_unlock(int sem_id) {
    struct sembuf sops[2] = {
        {0, 0, IPC_NOWAIT},  
        {0, 1, 0}          
    };
    
    if (semop(sem_id, sops, 2) == -1) {
        if (errno == EAGAIN) return; 
        perror("semop unlock");
        exit(EXIT_FAILURE);
    }
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

    
    key_t semKey;
    if ((semKey = ftok("fileSave", 159)) == (key_t) -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    int sem_id = semget(semKey, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    if (semctl(sem_id, 0, SETVAL, 1) == -1) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(FILE_NAME, "w");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    fclose(file);

    int pipefd[2];
    if (pipe(pipefd)) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        close(pipefd[0]);
        srand(time(NULL));

        for (int i = 0; i < num_count; i++) {
            sem_lock(sem_id);

            FILE *file = fopen(FILE_NAME, "r");
            if (file == NULL) {
                perror("Failed to open file");
                exit(EXIT_FAILURE);
            }
            int num;
            fscanf(file, "%d", &num);
            fclose(file);
            
            sem_unlock(sem_id);

            num = rand() % 100;
            if (write(pipefd[1], &num, sizeof(num)) != sizeof(num)) {
                perror("write");
                close(pipefd[1]);
                exit(EXIT_FAILURE);
            }
        }

        close(pipefd[1]);
        exit(EXIT_SUCCESS);
    } else {
        close(pipefd[1]);
        int output_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd == -1) {
            perror("open");
            close(pipefd[0]);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < num_count; i++) {
            int num;
            if (read(pipefd[0], &num, sizeof(num)) == -1) {
                perror("read failed");
                close(output_fd);
                close(pipefd[0]);
                exit(EXIT_FAILURE);
            }

            sem_lock(sem_id); 

            FILE *file = fopen(FILE_NAME, "a");
            if (file == NULL) {
                perror("Failed to open file");
                exit(EXIT_FAILURE);
            }
            fprintf(file, "%d\n", num);
            fclose(file);

            sem_unlock(sem_id);

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

        }

        close(pipefd[0]);
        close(output_fd);
        wait(NULL);

        if (semctl(sem_id, 0, IPC_RMID) == -1) {
            perror("semctl remove");
        }
    }

    return EXIT_SUCCESS;
}