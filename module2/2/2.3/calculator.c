#include <stdio.h>
#include <stdarg.h>
#include <math.h>

#include "calculator.h"

void register_command(Commands *commands, const char *name, double (*func)(int, double[]), int min_args, int max_args) {
    if (commands->num_commands >= MAX_COMMANDS) {
        fprintf(stderr, "Нельзя зарегистрировать команду.\n");
        return;
    }
    commands->commands[commands->num_commands++] = (Command){name, func, min_args, max_args};
}

double add(int count, double args[]) {
    double sum = 0;
    for (int i = 0; i < count; i++) 
        sum += args[i];
    return sum;
}

double sub(int count, double args[]) {
    double result = args[0];
    for (int i = 1; i < count; i++) 
        result -= args[i];
    return result;
}

double mul(int count, double args[]) {
    double product = 1;
    for (int i = 0; i < count; i++) 
        product *= args[i];
    return product;
}

double divide(int count, double args[]) {
    double result = args[0];
    for (int i = 1; i < count; i++) {
        if (args[i] == 0) {
            return NAN;
        }
        result /= args[i];
    }
    return result;
}

double power(int count, double args[]) {
    return pow(args[0], args[1]);
}

double square_root(int count, double args[]) {
    return sqrt(args[0]);
}