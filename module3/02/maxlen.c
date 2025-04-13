#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Использование: maxlen <str1> <str2> ...\n");
        return 1;
    }
    int max_len = strlen(argv[1]);
    char *max_str = argv[1];
    for (int i = 2; i < argc; i++) {
        int len = strlen(argv[i]);
        if (len > max_len) {
            max_len = len;
            max_str = argv[i];
        }
    }
    printf("%s (длина: %d)\n", max_str, max_len);
    return 0;
}