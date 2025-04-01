#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "calculator.h"

int main() {
    load_plugins("/home/nikonoff/eltex/module2/6/6.3/build/plugins");
    
    char input[256];
    while(1) {
        printf("\nДоступные операции: add, sub, mul, div, pow, sqrt, exit\n");
        printf("> ");
        if(!fgets(input, sizeof(input), stdin)) break;
        
        char *cmd_name = strtok(input, " \t\n");
        if(!cmd_name) continue;
        
        if(strcmp(cmd_name, "exit") == 0) break;
        
        const Command *cmd = find_command(cmd_name);
        if(!cmd) {
            printf("Неизвестная команда\n");
            continue;
        }
        
        char *token;
        double args[10];
        int arg_count = 0;
        while ((token = strtok(NULL, " \t\n")) && arg_count < 10) {
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
    cleanup_plugins();
    return 0;
}