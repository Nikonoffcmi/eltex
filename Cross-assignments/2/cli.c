#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define MAX_DRIVERS 10
#define MAX_EVENTS 10
#define BUFF_SIZE 256

typedef enum {
    AVAILABLE,
    BUSY
} DriverStatus;

typedef struct {
    pid_t pid;
    char cmd_pipe[BUFF_SIZE];
    char resp_pipe[BUFF_SIZE];
    int resp_fd;
    DriverStatus status;
    int remaining;
} DriverInfo;

DriverInfo drivers[MAX_DRIVERS];
int driver_count = 0;

int epoll_fd;
int epoll_timeout = -1;

void add_driver(pid_t pid, const char *cmd_pipe, const char *resp_pipe, int resp_fd) {
    if (driver_count >= MAX_DRIVERS) {
        fprintf(stderr, "Maximum drivers reached\n");
        return;
    }

    DriverInfo *driver = &drivers[driver_count++];
    driver->pid = pid;
    strcpy(driver->cmd_pipe, cmd_pipe);
    strcpy(driver->resp_pipe, resp_pipe);
    driver->resp_fd = resp_fd;
    driver->status = AVAILABLE;
    driver->remaining = 0;
}

DriverInfo* find_driver_by_pid(pid_t pid) {
    for (int i = 0; i < driver_count; i++) {
        if (drivers[i].pid == pid) {
            return &drivers[i];
        }
    }
    return NULL;
}

DriverInfo* find_driver_by_resp_fd(int fd) {
    for (int i = 0; i < driver_count; i++) {
        if (drivers[i].resp_fd == fd) {
            return &drivers[i];
        }
    }
    return NULL;
}

void print_prompt() {
    printf("\n> ");
    fflush(stdout);
}

void handle_user_input() {
    char buf[BUFF_SIZE];

    if (!fgets(buf, BUFF_SIZE, stdin)) {
        perror("fgets");
        exit(EXIT_FAILURE);
    }
    
    buf[strcspn(buf, "\n")] = '\0';

    char *cmd = strtok(buf, " ");
    if (cmd == NULL){
        print_prompt();
        return;
    } 

    if (strcmp(cmd, "create_driver") == 0) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            char driver_exe[] = "./driver";
            char cmd_pipe[BUFF_SIZE], resp_pipe[BUFF_SIZE];
            snprintf(cmd_pipe, sizeof(cmd_pipe), "/tmp/driver_%d_cmd", getpid());
            snprintf(resp_pipe, sizeof(resp_pipe), "/tmp/driver_%d_resp", getpid());
            execl(driver_exe, driver_exe, cmd_pipe, resp_pipe, NULL);
            perror("execl");
            exit(EXIT_FAILURE);
        } else {
            char cmd_pipe[BUFF_SIZE], resp_pipe[BUFF_SIZE];
            snprintf(cmd_pipe, sizeof(cmd_pipe), "/tmp/driver_%d_cmd", pid);
            snprintf(resp_pipe, sizeof(resp_pipe), "/tmp/driver_%d_resp", pid);
            
            if (mkfifo(cmd_pipe, 0666) < 0) {
                perror("mkfifo cmd_pipe");
                exit(EXIT_FAILURE);
            }
            if (mkfifo(resp_pipe, 0666) < 0) {
                perror("mkfifo resp_pipe");
                exit(EXIT_FAILURE);
            }

            int resp_fd = open(resp_pipe, O_RDONLY | O_NONBLOCK);
            if (resp_fd == -1) {
                perror("open resp_pipe");
                return;
            }

            struct epoll_event event;
            event.events = EPOLLIN | EPOLLET;
            event.data.fd = resp_fd;
            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, resp_fd, &event) == -1) {
                perror("epoll_ctl");
                close(resp_fd);
                return;
            }

            add_driver(pid, cmd_pipe, resp_pipe, resp_fd);
            printf("Driver created with PID %d\n", pid);
            print_prompt();
        }
    } else if (strcmp(cmd, "send_task") == 0) {
        char *pid_str = strtok(NULL, " ");
        char *timer_str = strtok(NULL, " ");

        if (!pid_str || !timer_str) {
            printf("Usage: send_task <pid> <task_timer>\n");
            return;
        }

        pid_t pid = atoi(pid_str);
        int task_timer = atoi(timer_str);
        DriverInfo *driver = find_driver_by_pid(pid);
        if (!driver) {
            printf("Driver %d not found\n", pid);
            return;
        }
        int cmd_fd = open(driver->cmd_pipe, O_WRONLY | O_NONBLOCK);
        if (cmd_fd == -1) {
            perror("open cmd_pipe");
            return;
        }

        if (dprintf(cmd_fd, "SEND_TASK %d\n", task_timer) < 0){
            perror("dprintf SEND_TASK");
            exit(EXIT_FAILURE);
        }
        close(cmd_fd);
    } else if (strcmp(cmd, "get_status") == 0) {
        char *pid_str = strtok(NULL, " ");
        if (!pid_str) {
            printf("Usage: get_status <pid>\n");
            return;
        }
        pid_t pid = atoi(pid_str);
        DriverInfo *driver = find_driver_by_pid(pid);
        if (!driver) {
            printf("Driver %d not found\n", pid);
            return;
        }
        int cmd_fd = open(driver->cmd_pipe, O_WRONLY | O_NONBLOCK);
        if (cmd_fd == -1) {
            perror("open cmd_pipe");
            return;
        }
        if (dprintf(cmd_fd, "GET_STATUS\n") < 0){
            perror("dprintf GET_STATUS");
            exit(EXIT_FAILURE);
        }
        close(cmd_fd);
    } else if (strcmp(cmd, "get_drivers") == 0) {
        printf("Drivers:\n");
        for (int i = 0; i < driver_count; i++) {
            const char *status = drivers[i].status == AVAILABLE ? "Available" : "Busy";
            printf("Driver %d status: %s\n", drivers[i].pid, status);
        }
        print_prompt();
    } else {
        printf("Unknown command: %s\n", cmd);
        print_prompt();
    }
}

void handle_response(int fd) {
    char buf[BUFF_SIZE];
    ssize_t len = read(fd, buf, BUFF_SIZE - 1);
    if (len < 0 && errno != EAGAIN) {
        perror("read response");
        return;
    }
    buf[len] = '\0';

    DriverInfo *driver = find_driver_by_resp_fd(fd);
    if (!driver) {
        fprintf(stderr, "Unknown response fd %d\n", fd);
        return;
    }

    char *msg = strtok(buf, "\n");
    while (msg != NULL) {
        char status_update[] = "STATUS_UPDATE ";
        char busy[] = "Busy ";
        char available[] = "Available";
        char accepted[] = "ACCEPTED";
        char status_prefix[] = "STATUS ";

        size_t status_update_len = strlen(status_update);
        size_t busy_len = strlen(busy);
        size_t status_prefix_len = strlen(status_prefix);

        if (strlen(msg) >= status_update_len && strncmp(msg, status_update, status_update_len) == 0) {
            char *status_ptr = msg + status_update_len;
            if (strncmp(status_ptr, busy, busy_len) == 0) {
                driver->status = BUSY;
                driver->remaining = atoi(status_ptr + busy_len);
            } else if (strcmp(status_ptr, available) == 0) {
                driver->status = AVAILABLE;
                driver->remaining = 0;
            }
        } else if (strlen(msg) >= busy_len && strncmp(msg, busy, busy_len) == 0) {
            driver->status = BUSY;
            driver->remaining = atoi(msg + busy_len);
            printf("Driver %d is busy with %d seconds remaining\n", driver->pid, driver->remaining);
            print_prompt();
        } else if (strcmp(msg, accepted) == 0) {
            printf("Task accepted by driver %d\n", driver->pid);
            print_prompt();
        } else if (strlen(msg) >= status_prefix_len && strncmp(msg, status_prefix, status_prefix_len) == 0) {
            char *status_response = msg + status_prefix_len;
            if (strncmp(status_response, busy, busy_len) == 0) {
                driver->status = BUSY;
                driver->remaining = atoi(status_response + busy_len);
            } else if (strcmp(status_response, available) == 0) {
                driver->status = AVAILABLE;
                driver->remaining = 0;
            }
            printf("Driver %d status: %s\n", driver->pid, status_response);
            print_prompt();
        }
        msg = strtok(NULL, "\n");
    }
}

int main() {
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = STDIN_FILENO;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event) == -1) {
        perror("epoll_ctl stdin");
        exit(EXIT_FAILURE);
    }

    struct epoll_event events[MAX_EVENTS];

    printf("--------------- Taxi manger ---------------\n");
    printf("commands: create_driver, send_task, get_status, get_drivers\n> ");
    fflush(stdout);
    while (1) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, epoll_timeout);
        if (n == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == STDIN_FILENO) {
                handle_user_input();
            } else {
                handle_response(events[i].data.fd);
            }
        }
    }

    return EXIT_SUCCESS;
}