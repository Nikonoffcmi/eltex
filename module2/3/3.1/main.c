#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

void mode_to_letters(mode_t mode, char *str) {
    strcpy(str, "---------");
    static mode_t bits[] = {
        S_IRUSR, S_IWUSR, S_IXUSR,
        S_IRGRP, S_IWGRP, S_IXGRP,
        S_IROTH, S_IWOTH, S_IXOTH
    };
    static char chars[] = "rwxrwxrwx";
    for (int i = 0; i < 9; i++) {
        if (mode & bits[i]) {
            str[i] = chars[i];
        }
    }
    str[9] = '\0';
}

int mode_to_octal(mode_t mode) {
    return mode & 0777; 
}

void mode_to_bits(mode_t mode, char *bits_str) {
    mode_t perms = mode & 0777;
    for (int i = 8; i >= 0; i--) {
        bits_str[8 - i] = (perms & (1 << i)) ? '1' : '0';
    }
    bits_str[9] = '\0';
}

mode_t parse_input_to_mode(const char *input) {
    if (isdigit(input[0])) {
        int octal;
        sscanf(input, "%o", &octal);
        return (mode_t)(octal & 0777);
    } else {
        if (strlen(input) != 9) {
            fprintf(stderr, "Недопустимый символьный формат. Должно быть 9 символов.\n");
            exit(EXIT_FAILURE);
        }
        mode_t mode = 0;
        static mode_t bits[] = {
            S_IRUSR, S_IWUSR, S_IXUSR,
            S_IRGRP, S_IWGRP, S_IXGRP,
            S_IROTH, S_IWOTH, S_IXOTH
        };
        for (int i = 0; i < 9; i++) {
            if (input[i] != '-') {
                mode |= bits[i];
            }
        }
        return mode;
    }
}

void apply_part(mode_t *mode, const char *part) {
    int op_pos = strcspn(part, "+-=");
    if (part[op_pos] == '\0') {
        fprintf(stderr, "Недопустимая часть модификации: %s\n", part);
        return;
    }

    char categories[10];
    strncpy(categories, part, op_pos);
    categories[op_pos] = '\0';
    char op = part[op_pos];
    const char *perms = part + op_pos + 1;

    int apply_user = 0, apply_group = 0, apply_others = 0;
    for (char *c = categories; *c; c++) {
        switch (*c) {
            case 'u': apply_user = 1; break;
            case 'g': apply_group = 1; break;
            case 'o': apply_others = 1; break;
            case 'a': apply_user = apply_group = apply_others = 1; break;
            default: fprintf(stderr, "Неизвестная категория: %c\n", *c); return;
        }
    }
    if (op_pos == 0)
        apply_user = apply_group = apply_others = 1;

    mode_t add_bits = 0;
    for (const char *p = perms; *p; p++) {
        char c = *p;
        if (apply_user) {
            if (c == 'r') add_bits |= S_IRUSR;
            else if (c == 'w') add_bits |= S_IWUSR;
            else if (c == 'x') add_bits |= S_IXUSR;
        }
        if (apply_group) {
            if (c == 'r') add_bits |= S_IRGRP;
            else if (c == 'w') add_bits |= S_IWGRP;
            else if (c == 'x') add_bits |= S_IXGRP;
        }
        if (apply_others) {
            if (c == 'r') add_bits |= S_IROTH;
            else if (c == 'w') add_bits |= S_IWOTH;
            else if (c == 'x') add_bits |= S_IXOTH;
        }
    }

    switch (op) {
        case '+':
            *mode |= add_bits;
            break;
        case '-':
            *mode &= ~add_bits;
            break;
        case '=': {
            mode_t mask = 0;
            if (apply_user) mask |= S_IRWXU;
            if (apply_group) mask |= S_IRWXG;
            if (apply_others) mask |= S_IRWXO;
            *mode &= ~mask;
            *mode |= add_bits;
            break;
        }
        default:
            fprintf(stderr, "Неизвестный оператор: %c\n", op);
            break;
    }
}

void display_permissions(mode_t mode) {
    char letters[10];
    mode_to_letters(mode, letters);
    int octal = mode_to_octal(mode);
    char bits[10];
    mode_to_bits(mode, bits);

    printf("Буквенная запись: %s\n", letters);
    printf("Числовая запись: %03o\n", octal);
    printf("Битовая запись: %s\n\n", bits);
}

int is_octal_input(const char *input) {
    for (int i = 0; input[i]; i++) {
        if (input[i] < '0' || input[i] > '7') {
            return 0;
        }
    }
    return 1;
}

int main() {
    int choice;
    mode_t current_mode = 0;
    char filename[256];
    char input[256];
    struct stat st;

    printf("1. Ввести права доступа\n2. Ввести путь к файлу\n3. Выход\nВыбрать: ");
    scanf("%d", &choice);
    getchar();

    if (choice == 1) {
        printf("Введите права доступа: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';
        current_mode = parse_input_to_mode(input);
    } else if (choice == 2) {
        printf("Введите путь к файлу: ");
        fgets(filename, sizeof(filename), stdin);
        filename[strcspn(filename, "\n")] = '\0';
        if (stat(filename, &st)) {
            perror("stat");
            return 1;
        }
        current_mode = st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    } else {
        return 0;
    }

    printf("\nИзначальные права доступа:\n");
    display_permissions(current_mode);

    while (1) {
        printf("Введите права доступа (например, 755, u+rwx,g-x) или 'exit': ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';
        if (strcmp(input, "exit") == 0) break;

        if (is_octal_input(input)) { 
            int new_mode;
            sscanf(input, "%o", &new_mode);
            current_mode = (new_mode & 0777);
        } else { 
            char *part = strtok(input, ",");
            while (part) {
                apply_part(&current_mode, part);
                part = strtok(NULL, ",");
            }
        }

        printf("Обновленные права доступа:\n");
        display_permissions(current_mode);
    }

    return 0;
}