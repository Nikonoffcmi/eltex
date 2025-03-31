#define MAX_COMMANDS 20
#define MAX_ARGS 10

typedef struct {
    const char *name;
    double (*func)(int, double[]);
    int min_args;
    int max_args;
} Command;

typedef struct {
    Command commands[MAX_COMMANDS];
    int num_commands;
} Commands;

void register_command(Commands *commands, const char *name, double (*func)(int, double[]), int min_args, int max_args);

double add(int count, double args[]);

double sub(int count, double args[]);

double mul(int count, double args[]);

double divide(int count, double args[]);

double power(int count, double args[]);

double square_root(int count, double args[]);