#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#define MSG_TYPE1 1
#define MSG_TYPE2 2
#define EXIT_TYPE 3

struct message {
    long mtype;
    char mtext[256];
};

int main() {
    key_t key = ftok("chat", 159);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    if (msgid == -1) {
        perror("msgget");
        return EXIT_FAILURE;
    }

    struct message msg;
    int running = 1;

    printf("Программа 1, 'exit' чтобы выйти.\n");

    printf("Введите сообщение: ");
    fgets(msg.mtext, sizeof(msg.mtext), stdin);
    msg.mtype = MSG_TYPE1;
    if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
        perror("msgsnd");
        return EXIT_FAILURE;
    }

    while (running) {
        if (msgrcv(msgid, &msg, sizeof(msg.mtext), 0, 0) == -1) {
            perror("msgrcv");
            return EXIT_FAILURE;
        }

        if (msg.mtype == EXIT_TYPE) {
            printf("Прекращения диалога.\n");
            if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
                perror("msgsnd");
                return EXIT_FAILURE;
            }
            running = 0;
        } else if (msg.mtype == MSG_TYPE2) {
            printf("Получено от программы 2: %s", msg.mtext);

            printf("Введите сообщение: ");
            fgets(msg.mtext, sizeof(msg.mtext), stdin);
            if (strcmp(msg.mtext, "exit\n") == 0) {
                msg.mtype = EXIT_TYPE;
                if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
                    perror("msgsnd");
                    return EXIT_FAILURE;
                }
                if (msgrcv(msgid, &msg, sizeof(msg.mtext), 0, 0) == -1) {
                    perror("msgrcv");
                    return EXIT_FAILURE;
                }
                running = 0;
            } else {
                msg.mtype = MSG_TYPE1;
                if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
                    perror("msgsnd");
                    return EXIT_FAILURE;
                }
            }
        } else {
            printf("Неизвестный тип сообщения: %ld\n", msg.mtype);
        }
    }

    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        return EXIT_FAILURE;
    }
    printf("Очередь удалена.\n");
    return 0;
}