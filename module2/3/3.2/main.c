#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

int parse_ip(const char *ip_str, uint32_t *ip) {
    unsigned int octets[4];
    int end_pos;
    if (sscanf(ip_str, "%u.%u.%u.%u%n", &octets[0], &octets[1], &octets[2], &octets[3], &end_pos) != 4) {
        return 0;
    }
    if (ip_str[end_pos] != '\0') {
        return 0;
    }
    for (int i = 0; i < 4; i++) {
        if (octets[i] > 255) {
            return 0;
        }
    }
    *ip = (octets[0] << 24) | (octets[1] << 16) | (octets[2] << 8) | octets[3];
    return 1;
}

int is_valid_mask(uint32_t mask) {
    uint32_t complement = ~mask;
    if (complement == 0) {
        return 1;
    }
    return (complement & (complement + 1)) == 0;
}

uint32_t generate_random_ip() {
    return (rand() % 256) << 24 |
            (rand() % 256) << 16 |
            (rand() % 256) << 8 |
            (rand() % 256);
}

int is_same_subnet(uint32_t gateway, uint32_t dest_ip, uint32_t mask) {
    return (gateway & mask) == (dest_ip & mask);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Использование: %s ip_шлюза маска_подсети N\n", argv[0]);
        return 1;
    }

    const char *gateway_str = argv[1];
    const char *mask_str = argv[2];
    int N = atoi(argv[3]);
    if (N <= 0) {
        fprintf(stderr, "N должно быть положительным целым числом\n");
        return 1;
    }

    uint32_t gateway, mask;
    if (!parse_ip(gateway_str, &gateway)) {
        fprintf(stderr, "Неверный IP-адрес шлюза\n");
        return 1;
    }
    if (!parse_ip(mask_str, &mask)) {
        fprintf(stderr, "Недопустимая маска подсети\n");
        return 1;
    }
    if (!is_valid_mask(mask)) {
        fprintf(stderr, "Недопустимая маска подсети (не непрерывная маска)\n");
        return 1;
    }

    srand(time(NULL));

    int same_count = 0;
    for (int i = 0; i < N; i++) {
        uint32_t dest_ip = generate_random_ip();
        if (is_same_subnet(gateway, dest_ip, mask)) {
            same_count++;
        }
    }
    int other_count = N - same_count;
    float same_percent = (float)same_count / N * 100;
    float other_percent = (float)other_count / N * 100;

    printf("Одна и та же подсеть: %d (%.2f%%)\n", same_count, same_percent);
    printf("Другие сети: %d (%.2f%%)\n", other_count, other_percent);

    return 0;
}