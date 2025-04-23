#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>

#define MAX_NUMBERS 1024
#define SHM_SIZE sizeof(struct shared_data)
#define SHM_NAME "/shm_name"

struct shared_data {
    sem_t sem_parent;
    sem_t sem_child;
    int count;
    int numbers[MAX_NUMBERS];
    int max;
    int min;
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

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    struct shared_data *shm = mmap(NULL, SHM_SIZE, 
                                    PROT_READ | PROT_WRITE,
                                    MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }
    close(shm_fd);

    if (sem_init(&shm->sem_parent, 1, 0) == -1 ||
        sem_init(&shm->sem_child, 1, 0) == -1) {
        perror("sem_init");
        munmap(shm, SHM_SIZE);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        sem_destroy(&shm->sem_parent);
        sem_destroy(&shm->sem_child);
        munmap(shm, SHM_SIZE);
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        while (1) {
            if (sem_wait(&shm->sem_child)) {
                perror("sem_wait child");
                exit(EXIT_FAILURE);
            }
            
            process_data(shm);
            
            if (sem_post(&shm->sem_parent)) {
                perror("sem_post child");
                exit(EXIT_FAILURE);
            }
        }
        exit(EXIT_SUCCESS);
    } else {
        int count = 0;
        while (!stop) {
            generate_data(shm);
            
            if (sem_post(&shm->sem_child)) {
                if (errno == EINTR && stop) break;
                perror("sem_post parent");
                break;
            }
            
            if (sem_wait(&shm->sem_parent)) {
                if (errno == EINTR && stop) break;
                perror("sem_wait parent");
                break;
            }
            
            printf("Max: %d, Min: %d\n", shm->max, shm->min);
            count++;
        }

        printf("Данных обработано: %d\n", count);

        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);

        sem_destroy(&shm->sem_parent);
        sem_destroy(&shm->sem_child);
        munmap(shm, SHM_SIZE);
        shm_unlink(SHM_NAME);
    }

    return EXIT_SUCCESS;
}