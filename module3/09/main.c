#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <time.h>

#define FILE_NAME "input.txt"
#define MAX_READERS 5

struct shared_data {
    int readers_count;
};

void sem_op(int sem_id, int sem_num, int op) {
    struct sembuf sops = {sem_num, op, 0};
    if (semop(sem_id, &sops, 1) == -1) {
        perror("semop");
        exit(EXIT_FAILURE);
    }
}

void writer_lock(int sem_id) {
    sem_op(sem_id, 1, -1);  
}

void writer_unlock(int sem_id) {
    struct sembuf sops[2] = {
        {1, 0, 0}, 
        {1, 1, 0}   
    };
    if (semop(sem_id, sops, 2) == -1) {
        perror("semop unlock");
        exit(EXIT_FAILURE);
    }
}

void reader_lock(int sem_id, struct shared_data *shm) {
    sem_op(sem_id, 0, -1);  
    shm->readers_count++;
    if (shm->readers_count == 1) {
        writer_lock(sem_id);
    }
    sem_op(sem_id, 0, 1);
}

void reader_unlock(int sem_id, struct shared_data *shm) {
    sem_op(sem_id, 0, -1);
    shm->readers_count--;
    if (shm->readers_count == 0) {
        writer_unlock(sem_id); 
    }
    sem_op(sem_id, 0, 1);
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <количество_чисел> <количество_читающих_процессов>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_count = atoi(argv[1]);
    int readers_count = atoi(argv[2]);
    if (num_count <= 0 || readers_count <= 0 || readers_count > MAX_READERS) {
        fprintf(stderr, "Неверное количество\n");
        exit(EXIT_FAILURE);
    }

    key_t semKey;
    if ((semKey = ftok("fileSave", 159)) == (key_t) -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    int sem_id = semget(semKey, 2, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    if (semctl(sem_id, 0, SETVAL, 1) == -1) {
        perror("semctl 0");
        exit(EXIT_FAILURE);
    }
    if (semctl(sem_id, 1, SETVAL, 1) == -1) {
        perror("semctl 1");
        exit(EXIT_FAILURE);
    }

    key_t shmKey;
    if ((shmKey = ftok("shmFile", 159)) == (key_t) -1) {
        perror("ftok shm");
        exit(EXIT_FAILURE);
    }
    int shm_id = shmget(shmKey, sizeof(struct shared_data), IPC_CREAT | 0666);
    struct shared_data *shm = shmat(shm_id, NULL, 0);
    shm->readers_count = 0;

    FILE *file = fopen(FILE_NAME, "w");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    fclose(file);

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < readers_count; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            close(pipefd[0]);
            srand(time(NULL) ^ getpid());

            for (int j = 0; j < num_count; j++) {
                reader_lock(sem_id, shm);

                FILE *file = fopen(FILE_NAME, "r");
                    if (file == NULL) {
                    perror("Failed to open file");
                    exit(EXIT_FAILURE);
                }
                int num;
                fscanf(file, "%d", &num);
                fclose(file);

                reader_unlock(sem_id, shm);

                num = rand() % 100;
                if (write(pipefd[1], &num, sizeof(num)) != sizeof(num)) {
                    perror("write");
                    close(pipefd[1]);
                    exit(EXIT_FAILURE);
                }
                sleep(1);
            }
            close(pipefd[1]);
            exit(EXIT_SUCCESS);
        }
    }

    close(pipefd[1]);
    int output_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd == -1) {
        perror("open");
        close(pipefd[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_count * readers_count; i++) {
        int num;
        if (read(pipefd[0], &num, sizeof(num)) == -1) {
            perror("read failed");
            close(output_fd);
            close(pipefd[0]);
            exit(EXIT_FAILURE);
        }

        writer_lock(sem_id);

        FILE *file = fopen(FILE_NAME, "a");
        if (file == NULL) {
            perror("Failed to open file");
            exit(EXIT_FAILURE);
        }
        fprintf(file, "%d\n", num);
        fclose(file);

        writer_unlock(sem_id);

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
    
    while (wait(NULL) > 0);

    shmdt(shm);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    semctl(sem_id, 1, IPC_RMID);

    return EXIT_SUCCESS;
}