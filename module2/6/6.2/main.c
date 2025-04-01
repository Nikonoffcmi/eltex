#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int main(int argc, char const *argv[])
{
    void *handle;
    void (*Menu)(void);
    char *error;

    handle = dlopen("/home/nikonoff/eltex/module2/6/6.2/build/libmanual.so", RTLD_LAZY);
    if (!handle)
    {
        fputs(dlerror(), stderr);
        exit(1);
    }

    Menu = dlsym(handle, "Menu");
    if ((error = dlerror()) != NULL)
    {
        fprintf(stderr, "%s\n", error);
        exit(-1);
    }

    Menu();
    dlclose(handle);

    return 0;
}