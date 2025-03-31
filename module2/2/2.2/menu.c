#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "menu.h"
#include "calculator.h"

void Menu() {
    char buffer[256];
    while (1) {
        printf("\nДоступные операции: add, sub, mul, div, pow, sqrt, exit\n");
        printf("> ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;

        char *operation = strtok(buffer, " \t\n");
        if (!operation) continue;

        if (strcmp(operation, "exit") == 0) break;

        double args[10];
        int count = 0;
        char *token;
        while ((token = strtok(NULL, " \t\n")) && count < 10) {
            char *endptr;
            double num = strtod(token, &endptr);
            if (*endptr != '\0') {
                printf("Неверный аргумент: %s\n", token);
                count = -1;
                break;
            }
            args[count++] = num;
        }

        if (count == -1) continue;

        double result = 0;
        if (strcmp(operation, "add") == 0) {
            if (count < 1) {
                printf("Для сложения необходим как минимум один аргумент.\n");
                continue;
            }
            switch(count) {
                case 1: result = add(1, args[0]); break;
                case 2: result = add(2, args[0], args[1]); break;
                case 3: result = add(3, args[0], args[1], args[2]); break;
                case 4: result = add(4, args[0], args[1], args[2], args[3]); break;
                case 5: result = add(5, args[0], args[1], args[2], args[3], args[4]); break;
                default: printf("Слишком много аргументов.\n"); continue;
            }
        } else if (strcmp(operation, "sub") == 0) {
            if (count < 2) {
                printf("Для вычитания необходимо как минимум два аргумента.\n");
                continue;
            }
            switch(count) {
                case 2: result = sub(2, args[0], args[1]); break;
                case 3: result = sub(3, args[0], args[1], args[2]); break;
                case 4: result = sub(4, args[0], args[1], args[2], args[3]); break;
                case 5: result = sub(5, args[0], args[1], args[2], args[3], args[4]); break;
                default: printf("Слишком много аргументов.\n"); continue;
            }
        } else if (strcmp(operation, "mul") == 0) {
            if (count < 1) {
                printf("Для умножения необходим как минимум один аргумент.\n");
                continue;
            }
            switch(count) {
                case 1: result = mul(1, args[0]); break;
                case 2: result = mul(2, args[0], args[1]); break;
                case 3: result = mul(3, args[0], args[1], args[2]); break;
                case 4: result = mul(4, args[0], args[1], args[2], args[3]); break;
                case 5: result = mul(5, args[0], args[1], args[2], args[3], args[4]); break;
                default: printf("Слишком много аргументов.\n"); continue;
            }
        } else if (strcmp(operation, "div") == 0) {
            if (count < 2) {
                printf("Для деления необходимо как минимум два аргумента.\n");
                continue;
            }
            switch(count) {
                case 2: result = divide(2, args[0], args[1]); break;
                case 3: result = divide(3, args[0], args[1], args[2]); break;
                case 4: result = divide(4, args[0], args[1], args[2], args[3]); break;
                case 5: result = divide(5, args[0], args[1], args[2], args[3], args[4]); break;
                default: printf("Слишком много аргументов.\n"); continue;
            }
        } else if (strcmp(operation, "pow") == 0) {
            if (count != 2) {
                printf("Для возведения в степень необходимо только два аргумента.\n");
                continue;
            }
            result = power(2, args[0], args[1]);
        } else if (strcmp(operation, "sqrt") == 0) {
            if (count != 1) {
                printf("Для получения квадратного корня необходимо только один аргумент.\n");
                continue;
            }
            result = square_root(1, args[0]);
        } else {
            printf("Неизвестная операция: %s\n", operation);
            continue;
        }

        if (isnan(result))
            printf("Ошибка в операции: %s\n", operation);
        else
            printf("Результат: %lf\n", result);
    }

}