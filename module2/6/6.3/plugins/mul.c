#include "calculator.h"

double calculate(int count, double args[]) {
    double product = 1;
    for (int i = 0; i < count; i++) 
        product *= args[i];
    return product;
}

Command command_export = {
    .name = "mul",
    .func = calculate,
    .min_args = 1,
    .max_args = 10
};