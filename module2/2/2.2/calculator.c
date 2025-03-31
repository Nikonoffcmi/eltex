#include <stdio.h>
#include <stdarg.h>
#include <math.h>

double add(int count, ...) {
    va_list args;
    va_start(args, count);
    double sum = 0;
    for (int i = 0; i < count; i++) {
        sum += va_arg(args, double);
    }
    va_end(args);
    return sum;
}

double sub(int count, ...) {
    va_list args;
    va_start(args, count);
    double result = va_arg(args, double);
    for (int i = 1; i < count; i++) {
        result -= va_arg(args, double);
    }
    va_end(args);
    return result;
}

double mul(int count, ...) {
    va_list args;
    va_start(args, count);
    double product = 1;
    for (int i = 0; i < count; i++) {
        product *= va_arg(args, double);
    }
    va_end(args);
    return product;
}

double divide(int count, ...) {
    va_list args;
    va_start(args, count);
    double result = va_arg(args, double);
    for (int i = 1; i < count; i++) {
        double divisor = va_arg(args, double);
        if (divisor == 0) {
            // printf("Ошибка: Деление на нуль.\n");
            return NAN;
        }
        result /= divisor;
    }
    va_end(args);
    return result;
}

double power(int count, ...) {
    if (count != 2) return NAN;
    va_list args;
    va_start(args, count);
    double base = va_arg(args, double);
    double exponent = va_arg(args, double);
    va_end(args);
    return pow(base, exponent);
}

double square_root(int count, ...) {
    if (count != 1) return NAN;
    va_list args;
    va_start(args, count);
    double num = va_arg(args, double);
    va_end(args);
    return sqrt(num);
}
