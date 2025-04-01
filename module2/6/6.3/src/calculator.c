#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "calculator.h"

#define MAX_COMMANDS 20
static Command commands[MAX_COMMANDS];
static int num_commands = 0;

void register_command(const char *name, 
                    double (*func)(int, double[]), 
                    int min_args, 
                    int max_args,
                    void* handle) {
    if(num_commands >= MAX_COMMANDS) {
        fprintf(stderr, "Достигнут лимит команд\n");
        return;
    }
    commands[num_commands++] = (Command){
        .name = name,
        .func = func,
        .min_args = min_args,
        .max_args = max_args,
        .handle = handle
    };
}

void load_plugins(const char *dirpath) {
    DIR *dir = opendir(dirpath);
    if(!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while((entry = readdir(dir))) {
        if(entry->d_type == DT_REG && 
           strstr(entry->d_name, ".so")) {
            char path[256];
            snprintf(path, sizeof(path), 
                    "%s/%s", dirpath, entry->d_name);
            
            void *handle = dlopen(path, RTLD_LAZY);
            if(!handle) {
                fprintf(stderr, "Ошибка загрузки %s: %s\n", 
                       entry->d_name, dlerror());
                continue;
            }

            Command *cmd = dlsym(handle, "command_export");
            if(cmd) {
                register_command(cmd->name, cmd->func, 
                                cmd->min_args, cmd->max_args, handle);
            }
            else {
                dlclose(handle);
            }
        }
    }
    closedir(dir);
}

const Command* find_command(const char *name) {
    for(int i = 0; i < num_commands; i++) {
        if(strcmp(commands[i].name, name) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}

void cleanup_plugins() {
    for(int i = 0; i < num_commands; i++) {
        if(commands[i].handle) {
            dlclose(commands[i].handle);
        }
    }
    num_commands = 0;
}