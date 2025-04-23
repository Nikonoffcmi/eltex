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

    printf("Программа 2, 'exit' чтобы выйти.\n");

    while (1) {
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
            return EXIT_SUCCESS;
        } else if (msg.mtype == MSG_TYPE1) {
            printf("Получено от программы 1: %s", msg.mtext);

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
                return EXIT_SUCCESS;
            } else {
                msg.mtype = MSG_TYPE2;
                if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
                    perror("msgsnd");
                    return EXIT_FAILURE;
                }
            }
        } else {
            printf("Неизвестный тип сообщения: %ld\n", msg.mtype);
        }
    }

    return 0;
}