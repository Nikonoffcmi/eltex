#include "calculator.h"
#include <math.h>

double calculate(int count, double args[]) {
    return pow(args[0], args[1]);
}

Command command_export = {
    .name = "pow",
    .func = calculate,
    .min_args = 2,
    .max_args = 2
};