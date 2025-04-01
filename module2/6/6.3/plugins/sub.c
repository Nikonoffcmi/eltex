#include "calculator.h"

double calculate(int count, double args[]) {
    double result = args[0];
    for (int i = 1; i < count; i++) 
        result -= args[i];
    return result;
}

Command command_export = {
    .name = "sub",
    .func = calculate,
    .min_args = 2,
    .max_args = 10
};