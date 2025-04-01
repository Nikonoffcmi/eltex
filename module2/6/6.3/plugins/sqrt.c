#include "calculator.h"
#include <math.h>

double calculate(int count, double args[]) {
    return sqrt(args[0]);
}

__attribute__((visibility("default")))
Command command_export = {
    .name = "sqrt",
    .func = calculate,
    .min_args = 1,
    .max_args = 1
};