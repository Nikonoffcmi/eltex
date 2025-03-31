#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "menu.h"
#include "calculator.h"

void Menu() {
    Commands commands;
    register_command(&commands, "add", add, 1, MAX_ARGS);
    register_command(&commands, "sub", sub, 2, MAX_ARGS);
    register_command(&commands, "mul", mul, 1, MAX_ARGS);
    register_command(&commands, "div", divide, 2, MAX_ARGS);
    register_command(&commands, "pow", power, 2, 2);
    register_command(&commands, "sqrt", square_root, 1, 1);

    char buffer[256];
    while (1) {
        printf("\nДоступные операции: add, sub, mul, div, pow, sqrt, exit\n");
        printf("> ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;

        char *token = strtok(buffer, " \t\n");
        if (!token) continue;

        if (strcmp(token, "exit") == 0) break;

        const Command *cmd = NULL;
        for (int i = 0; i < commands.num_commands; i++) {
            if (strcmp(commands.commands[i].name, token) == 0) {
                printf("Неизвестная команда.\n");
                cmd = &commands.commands[i];
                break;
            }
        }

        if (!cmd) {
            printf("Неизвестная команда.\n");
            continue;
        }

        double args[MAX_ARGS];
        int arg_count = 0;
        while ((token = strtok(NULL, " \t\n")) && arg_count < MAX_ARGS) {
            char *endptr;
            args[arg_count] = strtod(token, &endptr);
            if (*endptr != '\0') {
                printf("Неверный аргумент: %s\n", token);
                arg_count = -1;
                break;
            }
            arg_count++;
        }

        if (arg_count == -1) continue;

        if (arg_count < cmd->min_args || arg_count > cmd->max_args) {
            printf("%s требует от %d до %d аргументов\n", 
                    cmd->name, cmd->min_args, cmd->max_args);
            continue;
        }

        double result = cmd->func(arg_count, args);
        if (isnan(result))
            printf("Ошибка в операции: %s\n", cmd->name);
        else
            printf("Результат: %lf\n", result);

    }

}