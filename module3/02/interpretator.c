#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_INPUT 256
#define MAX_ARGS 64

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    char cmd_path[MAX_INPUT];

    while (1) {
        printf("Команды: sum max concat maxlen \n");
        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = 0;

        int arg_count = 0;
        char *token = strtok(input, " \t");
        if (!token) continue;
        if (strcmp(token, "exit") == 0) break;
        
        while (token != NULL && arg_count < MAX_ARGS - 1) {
            args[arg_count++] = token;
            token = strtok(NULL, " \t");
        }
        args[arg_count] = NULL;

        if (arg_count == 0) {
            continue;
        }

        snprintf(cmd_path, sizeof(cmd_path), "./%s", args[0]);
        if (access(cmd_path, X_OK) != 0) {
            fprintf(stderr, "Ошибка: Команда '%s' не найдена.\n", args[0]);
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {
            execv(cmd_path, args);
            perror("execv");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("fork");
        } else {
            int status;
            waitpid(pid, &status, 0);
            if (!WIFEXITED(status))
            {
                printf("child not exited");
                exit(0);
            }
        }

        printf("\n");

    }

    return 0;
}