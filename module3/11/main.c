#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#define FILE_NAME "input.txt"
#define MAX_READERS 5
#define SEM_MUTEX_NAME "/mutex_sem"
#define SEM_WRITE_NAME "/write_sem"

typedef struct {
    sem_t *mutex;
    sem_t *write_lock;
    int readers_count;
} shared_data;

void cleanup_resources(shared_data *data, int pipefd[], int output_fd) {
    if (data) {
        if (data->mutex) {
            sem_close(data->mutex);
            sem_unlink(SEM_MUTEX_NAME);
        }
        if (data->write_lock) {
            sem_close(data->write_lock);
            sem_unlink(SEM_WRITE_NAME);
        }
        munmap(data, sizeof(shared_data));
    }
    if (pipefd[0] != -1) close(pipefd[0]);
    if (pipefd[1] != -1) close(pipefd[1]);
    if (output_fd != -1) close(output_fd);
}

void reader_lock(shared_data *data) {
    if (sem_wait(data->mutex) == -1) {
        perror("sem_wait reader_lock");
        exit(EXIT_FAILURE);
    }
    data->readers_count++;
    if(data->readers_count == 1) {
        if (sem_wait(data->write_lock) == -1) {
            perror("sem_wait write_lock reader_lock");
            exit(EXIT_FAILURE);
        }
    }
    if (sem_post(data->mutex) == -1) {
        perror("sem_post reader_lock");
        exit(EXIT_FAILURE);
    }
}

void reader_unlock(shared_data *data) {
    if (sem_wait(data->mutex) == -1) {
        perror("sem_wait reader_unlock");
        exit(EXIT_FAILURE);
    }
    data->readers_count--;
    if (data->readers_count == 0) {
        if (sem_post(data->write_lock) == -1) {
            perror("sem_post write_lock reader_unlock");
            exit(EXIT_FAILURE);
        }
    }
    if (sem_post(data->mutex) == -1) {
        perror("sem_post reader_unlock");
        exit(EXIT_FAILURE);
    }
}

void writer_lock(shared_data *data) {
    if (sem_wait(data->write_lock) == -1) {
        perror("sem_wait write_lock writer_lock");
        exit(EXIT_FAILURE);
    }
}

void writer_unlock(shared_data *data) {
    if (sem_post(data->write_lock) == -1) {
        perror("sem_post write_lock writer_unlock");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    int output_fd = -1;
    int pipefd[2] = {-1, -1};
    shared_data *data = NULL;
    
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

    data = mmap(NULL, sizeof(shared_data), PROT_READ | PROT_WRITE, 
                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (data == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }
    data->readers_count = 0;

    data->mutex = sem_open(SEM_MUTEX_NAME, O_CREAT | O_EXCL, 0666, 1);
    if (data->mutex == SEM_FAILED) {
        perror("sem_open failed");
        cleanup_resources(data, pipefd, output_fd);
        exit(EXIT_FAILURE);
    }
    data->write_lock = sem_open(SEM_WRITE_NAME, O_CREAT | O_EXCL, 0666, 1);
    if (data->write_lock == SEM_FAILED) {
        perror("sem_open write_lock failed");
        cleanup_resources(data, pipefd, output_fd);
        exit(EXIT_FAILURE);
    }
    
    FILE *file = fopen(FILE_NAME, "w");
    if (file == NULL) {
        perror("Failed to open file");
        cleanup_resources(data, pipefd, output_fd);
        exit(EXIT_FAILURE);
    }
    fclose(file);

    if (pipe(pipefd) == -1) {
        perror("pipe");
        cleanup_resources(data, pipefd, output_fd);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < readers_count; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            close(pipefd[0]);
            srand(time(NULL) ^ getpid());

            for (int j = 0; j < num_count; j++) {
                reader_lock(data);

                FILE *file = fopen(FILE_NAME, "r");
                if (file == NULL) {
                    perror("Failed to open file");
                    cleanup_resources(data, pipefd, output_fd);
                    exit(EXIT_FAILURE);
                }
                int num;
                fscanf(file, "%d", &num);
                fclose(file);

                reader_unlock(data);

                num = rand() % 100;
                if (write(pipefd[1], &num, sizeof(num)) != sizeof(num)) {
                    perror("write");
                    cleanup_resources(data, pipefd, output_fd);
                    exit(EXIT_FAILURE);
                }
                sleep(1);
            }
            close(pipefd[1]);
            exit(EXIT_SUCCESS);
        }
    }

    close(pipefd[1]);
    output_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd == -1) {
        perror("open");
        cleanup_resources(data, pipefd, output_fd);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_count * readers_count; i++) {
        int num;
        if (read(pipefd[0], &num, sizeof(num)) == -1) {
            perror("read failed");
            cleanup_resources(data, pipefd, output_fd);
            exit(EXIT_FAILURE);
        }

        writer_lock(data);

        FILE *file = fopen(FILE_NAME, "a");
        if (file == NULL) {
            perror("Failed to open file");
            cleanup_resources(data, pipefd, output_fd);
            exit(EXIT_FAILURE);
        }
        fprintf(file, "%d\n", num);
        fclose(file);

        writer_unlock(data);

        char buf[32];
        int len = snprintf(buf, sizeof(buf), "%d\n", num);
        if (write(STDOUT_FILENO, buf, len) != len) {
            perror("write stdout");
            cleanup_resources(data, pipefd, output_fd);
            exit(EXIT_FAILURE);
        }
        if (write(output_fd, buf, len) != len) {
            perror("write file");
            cleanup_resources(data, pipefd, output_fd);
            exit(EXIT_FAILURE);
        }

    }

    cleanup_resources(data, pipefd, output_fd);

    int status;
    while (wait(&status) > 0) {
        if (!WIFEXITED(status)) {
            fprintf(stderr, "Child process exited with error\n");
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}