#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char * argv[]) {
    char * const compiler = getenv("PROXIED_NATIVE_CXX");

    if (compiler == NULL) {
        fprintf(stderr, "PROXIED_NATIVE_CXX environment variable is not set.\n");
        return 1;
    }

    if (compiler[0] == '\0') {
        fprintf(stderr, "PROXIED_NATIVE_CXX environment variable value should be a non-empty string.\n");
        return 2;
    }

    char* args[argc + 2];

    args[0] = compiler;

    for (int i = 1; i < argc; i++) {
        args[i] = argv[i];
    }

    char * const sysroot = getenv("SYSROOT_FOR_BUILD");

    if (sysroot == NULL || sysroot[0] == '\0') {
        args[argc] = NULL;
    } else {
        args[argc]     = (char*)"-isysroot";
        args[argc + 1] = sysroot;
        args[argc + 2] = NULL;
    }

    execv (compiler, args);
    perror(compiler);
    return 255;
}
