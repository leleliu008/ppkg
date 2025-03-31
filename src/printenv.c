#include <stdio.h>

#ifdef __APPLE__
#include <crt_externs.h>
#else
#include <unistd.h>
#endif

void printenv() {
#ifdef __APPLE__
    char **envlist = *_NSGetEnviron();
#else
    char **envlist = environ;
#endif

    for (int i = 0; envlist[i] != NULL; i++) {
        puts(envlist[i]);
    }
}
