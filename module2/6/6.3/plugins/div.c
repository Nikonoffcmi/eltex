#include "calculator.h"
#include <math.h>

double calculate(int count, double args[]) {
    double result = args[0];
    for (int i = 1; i < count; i++) {
        if (args[i] == 0) {
            return NAN;
        }
        result /= args[i];
    }
    return result;
}

Command command_export = {
    .name = "div",
    .func = calculate,
    .min_args = 2,
    .max_args = 10
};