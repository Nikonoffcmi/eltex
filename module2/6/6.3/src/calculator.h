typedef struct {
    const char *name;
    double (*func)(int, double[]);
    int min_args;
    int max_args;
    void* handle;
} Command;

void register_command(const char *name, 
                    double (*func)(int, double[]), 
                    int min_args, 
                    int max_args,
                    void* handle);
void load_plugins(const char *dirpath);
const Command* find_command(const char *name);
void cleanup_plugins();