#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#define BUF_SIZE 256

typedef enum {
    AVAILABLE,
    BUSY
} State;

void reopen_cmd_pipe(int* cmd_fd, int epoll_fd, const char* cmd_pipe) {
    if (*cmd_fd != -1) {
        close(*cmd_fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, *cmd_fd, NULL);
    }
    
    *cmd_fd = open(cmd_pipe, O_RDONLY | O_NONBLOCK);
    if (*cmd_fd == -1) {
        perror("open cmd_pipe");
        exit(EXIT_FAILURE);
    }

    struct epoll_event event = {
        .events = EPOLLIN | EPOLLHUP,
        .data.fd = *cmd_fd
    };
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, *cmd_fd, &event) == -1) {
        perror("epoll_ctl cmd_fd");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <cmd_pipe> <resp_pipe>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *cmd_pipe = argv[1];
    const char *resp_pipe = argv[2];

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timer_fd == -1) {
        perror("timerfd_create");
        exit(EXIT_FAILURE);
    }

    State state = AVAILABLE;
    struct itimerspec timer_spec = {0};
    int cmd_fd = -1;
    int resp_fd = -1;

    
    reopen_cmd_pipe(&cmd_fd, epoll_fd, cmd_pipe);

    resp_fd = open(resp_pipe, O_WRONLY | O_NONBLOCK);
    if (resp_fd == -1) {
        perror("open resp_pipe");
        exit(EXIT_FAILURE);
    }

    struct epoll_event event = {
        .events = EPOLLIN,
        .data.fd = timer_fd
    };
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &event) == -1) {
        perror("epoll_ctl timer_fd");
        exit(EXIT_FAILURE);
    }


    while (1) {
        struct epoll_event events[2];
        int n = epoll_wait(epoll_fd, events, 2, -1);
        if (n == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (fd == cmd_fd) {
                if (events[i].events & EPOLLHUP) {
                    reopen_cmd_pipe(&cmd_fd, epoll_fd, cmd_pipe);
                    continue;
                }

                char buf[BUF_SIZE];
                ssize_t len = read(cmd_fd, buf, BUF_SIZE - 1);
                if (len == 0) {
                    reopen_cmd_pipe(&cmd_fd, epoll_fd, cmd_pipe);
                    continue;
                }
                
                if (len < 0) {
                    if (errno == EAGAIN) continue;
                    perror("read cmd_fd");
                    continue;
                }

                buf[len] = '\0';

                char *msg = strtok(buf, "\n");
                while (msg != NULL) {
                    char send_task[] = "SEND_TASK ";
                    size_t send_task_len = strlen(send_task);
                    
                    if (strlen(msg) >= send_task_len && strncmp(msg, send_task, send_task_len) == 0) {
                        int task_timer = atoi(msg + send_task_len);
                        if (state == AVAILABLE) {
                            timer_spec.it_value.tv_sec = task_timer;
                            timer_spec.it_value.tv_nsec = 0;
                            timer_spec.it_interval.tv_sec = 0;
                            timer_spec.it_interval.tv_nsec = 0;
                            if (timerfd_settime(timer_fd, 0, &timer_spec, NULL) == -1) {
                                perror("timerfd_settime");
                            }
                            state = BUSY;
                            
                            if (dprintf(resp_fd, "ACCEPTED\n") < 0){
                                perror("timerfd_settime");
                                exit(EXIT_FAILURE);
                            }
                            if (dprintf(resp_fd, "STATUS_UPDATE Busy %d\n", task_timer) < 0){
                                perror("timerfd_settime");
                                exit(EXIT_FAILURE);
                            }
                        } else {
                            uint64_t exp;
                            read(timer_fd, &exp, sizeof(exp));
                            struct itimerspec curr;
                            timerfd_gettime(timer_fd, &curr);
                            int remaining = curr.it_value.tv_sec;
                            if (curr.it_value.tv_nsec > 0) remaining++;
                            if (dprintf(resp_fd, "Busy %d\n", remaining) < 0){
                                perror("timerfd_settime");
                                exit(EXIT_FAILURE);
                            }
                        }
                    } else if (strcmp(msg, "GET_STATUS") == 0) {
                        if (state == AVAILABLE) {
                            if (dprintf(resp_fd, "STATUS Available\n") < 0){
                                perror("timerfd_settime");
                                exit(EXIT_FAILURE);
                            }
                        } else {
                            struct itimerspec curr;
                            timerfd_gettime(timer_fd, &curr);
                            int remaining = curr.it_value.tv_sec;
                            if (curr.it_value.tv_nsec > 0) remaining++;
                            if (dprintf(resp_fd, "STATUS Busy %d\n", remaining) < 0){
                                perror("timerfd_settime");
                                exit(EXIT_FAILURE);
                            }
                        }
                    }
                    msg = strtok(NULL, "\n");
                }
            } else if (fd == timer_fd) {
                uint64_t exp;
                read(timer_fd, &exp, sizeof(exp));
                state = AVAILABLE;
                if (dprintf(resp_fd, "STATUS_UPDATE Available\n") < 0){
                    perror("timerfd_settime");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    close(cmd_fd);
    close(resp_fd);
    close(timer_fd);
    return 0;
}