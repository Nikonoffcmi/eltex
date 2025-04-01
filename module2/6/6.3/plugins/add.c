#include "calculator.h"

double calculate(int count, double args[]) {
    double sum = 0;
    for (int i = 0; i < count; i++) 
        sum += args[i];
    return sum;
}

Command command_export = {
    .name = "add",
    .func = calculate,
    .min_args = 1,
    .max_args = 10
};