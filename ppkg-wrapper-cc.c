#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>

//  ┌──────────────┐
//  │  preprocess  │
//  └──────────────┘
//         ↓
//  ┌──────────────┐
//  │   compile    │
//  └──────────────┘
//         ↓
//  ┌──────────────┐
//  │   assemble   │
//  └──────────────┘
//         ↓
//  ┌──────────────┐
//  │     link     │
//  └──────────────┘


#define ACTION_CREATE_OBJECT_FILE 1
#define ACTION_CREATE_SHARED_LIBRARY 2
#define ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE 3
#define ACTION_CREATE_DYNAMICALLY_LINKED_EXECUTABLE 4

int main(int argc, char* argv[]) {
    const char * options[6] = { "-shared", "-static", "--static", "-pie", "-c", "-o" };
          int    indexes[6] = {    -1,         -1,        -1,       -1,    -1,   -1  };

    for (int i = 1; i < argc; i++) {
        for (int j = 0; j < 6; j++) {
            if (strcmp(argv[i], options[j]) == 0) {
                indexes[j] = i;
                break;
            }
        }
    }

    int action = 0;

    if (indexes[0] > 0) {
        // if -shared option is specified, then remove -static , --static , -pie options if they also are specified
        action = ACTION_CREATE_SHARED_LIBRARY;
    } else if ((indexes[1] > 0) || (indexes[2] > 0)) {
        // if -shared option is not specified, but -static or --static option is specified, then remove -pie , -Wl,-Bdynamic option if it also is specified
        action = ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE;
    } else if (indexes[3] > 0) {
        action = ACTION_CREATE_DYNAMICALLY_LINKED_EXECUTABLE;
    }

    /////////////////////////////////////////////////////////////////

#ifdef WRAPPER_CXX
    char * const compiler = getenv("PROXIED_CXX");

    if (compiler == NULL) {
        fprintf(stderr, "PROXIED_CXX environment variable is not set.\n");
        return 1;
    }

    if (compiler[0] == '\0') {
        fprintf(stderr, "PROXIED_CXX environment variable value should be a non-empty string.\n");
        return 2;
    }
#else
    char * const compiler = getenv("PROXIED_CC");

    if (compiler == NULL) {
        fprintf(stderr, "PROXIED_CC environment variable is not set.\n");
        return 1;
    }

    if (compiler[0] == '\0') {
        fprintf(stderr, "PROXIED_CC environment variable value should be a non-empty string.\n");
        return 2;
    }
#endif

    /////////////////////////////////////////////////////////////////

    char* argv2[argc + 2];

    argv2[0] = compiler;

    if (action == ACTION_CREATE_SHARED_LIBRARY) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-static") == 0) {
                argv2[i] = (char*)"--shared";
            } else if (strcmp(argv[i], "--static") == 0) {
                argv2[i] = (char*)"--shared";
            } else if (strcmp(argv[i], "-pie") == 0) {
                argv2[i] = (char*)"--shared";
            } else {
                argv2[i] = argv[i];
            }
        }
    } else if (action == ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-Wl,--export-dynamic") == 0) {
                argv2[i] = (char*)"-static";
            } else if (strcmp(argv[i], "-Wl,-Bdynamic") == 0) {
                argv2[i] = (char*)"-static";
            } else if (strcmp(argv[i], "-pie") == 0) {
                argv2[i] = (char*)"-static";
            } else {
                argv2[i] = argv[i];
            }
        }
    } else {
        for (int i = 1; i < argc; i++) {
            argv2[i] = argv[i];
        }
    }

    /////////////////////////////////////////////////////////////////

    const char * const SYSROOT = getenv("SYSROOT");

    if (SYSROOT == NULL || SYSROOT[0] == '\0') {
        argv2[argc] = NULL;

        for (int i = 0; argv2[i] != NULL; i++) {
            printf("%s ", argv2[i]);
        }
        printf("\n");

        execv (compiler, argv2);
        perror(compiler);
        return 255;
    } else {
        size_t   sysrootArgLength = strlen(SYSROOT) + 11U;
        char     sysrootArg[sysrootArgLength];
        snprintf(sysrootArg, sysrootArgLength, "--sysroot=%s", SYSROOT);

        argv2[argc]     = sysrootArg;
        argv2[argc + 1] = NULL;

        for (int i = 0; argv2[i] != NULL; i++) {
            printf("%s ", argv2[i]);
        }
        printf("\n");

        execv (compiler, argv2);
        perror(compiler);
        return 255;
    }
}
