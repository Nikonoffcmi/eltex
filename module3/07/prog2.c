#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>

#define QUEUE_NAME "/chat_queue"
#define MAX_MSG_SIZE 256
#define MSG_PRIO_NORMAL 1
#define EXIT_PRIORITY 3

struct message {
    char text[MAX_MSG_SIZE];
};

int main() {
    mqd_t mq;
    struct message msg;
    unsigned int prio;
    int running = 1;

    mq = mq_open(QUEUE_NAME, O_RDWR);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        exit(1);
    }

    printf("Программа 2, 'exit' чтобы выйти.\n");

    while (running) {
        if (mq_receive(mq, (char*)&msg, sizeof(msg), &prio) == -1) {
            perror("mq_receive");
            break;
        }

        if (prio == EXIT_PRIORITY) {
            printf("Прекращения диалога.\n");
            running = 0;
        } else {
            printf("Получено от программы 1: %s", msg.text);

            printf("Введите сообщение: ");
            fgets(msg.text, MAX_MSG_SIZE, stdin);
            
            if (strcmp(msg.text, "exit\n") == 0) {
                if (mq_send(mq, (char*)&msg, sizeof(msg), EXIT_PRIORITY) == -1) {
                    perror("mq_send");
                }
                running = 0;
            } else {
                if (mq_send(mq, (char*)&msg, sizeof(msg), MSG_PRIO_NORMAL) == -1) {
                    perror("mq_send");
                }
            }
        }
    }

    mq_close(mq);
    return 0;
}