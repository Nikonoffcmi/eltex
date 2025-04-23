#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>

#define MAX_NUMBERS 1024
#define SHM_SIZE sizeof(struct shared_data)

struct shared_data {
    int count;
    int numbers[MAX_NUMBERS];
    int max;
    int min;
};

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

volatile sig_atomic_t stop = 0;

void handle_sigint(int sig) {
    stop = 1;
}

void generate_data(struct shared_data *shm) {
    shm->count = rand() % MAX_NUMBERS + 1;
    for (int i = 0; i < shm->count; i++) {
        shm->numbers[i] = rand();
    }
}

void process_data(struct shared_data *shm) {
    if (shm->count == 0) return;
    
    shm->max = shm->numbers[0];
    shm->min = shm->numbers[0];
    
    for (int i = 1; i < shm->count; i++) {
        if (shm->numbers[i] > shm->max) {
            shm->max = shm->numbers[i];
        }
        if (shm->numbers[i] < shm->min) {
            shm->min = shm->numbers[i];
        }
    }
}

int main() {
    signal(SIGINT, handle_sigint);
    srand(time(NULL));

    key_t shmKey;
    if ((shmKey = ftok("shmFile", 159)) == (key_t) -1) {
        perror("ftok shm");
        exit(EXIT_FAILURE);
    }
    int shmid = shmget(shmKey, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    struct shared_data *shm = shmat(shmid, NULL, 0);
    if (shm == (void *)-1) {
        perror("shmat");
        shmctl(shmid, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    key_t semKey;
    if ((semKey = ftok("fileSave", 159)) == (key_t) -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    int semid = semget(semKey, 2, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        shmdt(shm);
        shmctl(shmid, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    union semun arg;
    arg.val = 0;
    if (semctl(semid, 0, SETVAL, arg) == -1 || semctl(semid, 1, SETVAL, arg) == -1) { 
        perror("semctl");
        shmdt(shm);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID, arg);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        shmdt(shm);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID, arg);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        while (1) {
            struct sembuf sops = {0, -1, 0};
            
            if (semop(semid, &sops, 1) == -1) {
                perror("semop child wait");
                exit(EXIT_FAILURE);
            }

            process_data(shm);

            struct sembuf sops_read[2] = {
                {1, 0, 0}, 
                {1, 1, 0}   
            };
            if (semop(semid, sops_read, 2) == -1) {
                perror("semop child signal");
                exit(EXIT_FAILURE);
            }
        }
        exit(EXIT_SUCCESS);
    } else {
        int count = 0;
        while (!stop) {
            generate_data(shm);

            struct sembuf sops_write[2] = {
                {0, 0, 0},
                {0, 1, 0}
            };
            if (semop(semid, sops_write, 2) == -1) {
                if (errno == EINTR && stop) break;
                perror("semop parent signal");
                break;
            }

            struct sembuf sops = {1, -1, 0};
            if (semop(semid, &sops, 1) == -1) {
                if (errno == EINTR && stop) break;
                perror("semop parent wait");
                break;
            }

            printf("Max: %d, Min: %d\n", shm->max, shm->min);
            count++;
        }

        printf("Данных обработано: %d\n", count);

        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);

        shmdt(shm);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID, arg);
    }

    return EXIT_SUCCESS;
}