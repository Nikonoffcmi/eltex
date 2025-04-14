#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <сторона1> <сторона2> ...\n", argv[0]);
        return EXIT_FAILURE;
    }

    int total = argc - 1;
    int parent_count = (total + 1) / 2;

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return EXIT_FAILURE;
    }

    if (pid == 0) { 
        for (int i = parent_count + 1; i <= total; i++) {
            double side = atof(argv[i]);
            printf("Child (PID=%d): Площадь квадрата со стороной %.2f равна %.2f\n", 
                   getpid(), side, side * side);
        }
    } else {
        for (int i = 1; i <= parent_count; i++) {
            double side = atof(argv[i]);
            printf("Parent (PID=%d): Площадь квадрата со стороной %.2f равна %.2f\n", 
                   getpid(), side, side * side);
        }
        int status;
        wait(&status);
        if (!WIFEXITED(status))
        {
            printf("child not exited");
            exit(0);
        }
    }

    return EXIT_SUCCESS;
}