#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <regex.h>

#define ACTION_COMPILE 1
#define ACTION_CREATE_SHARED_LIBRARY 2
#define ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE 3
#define ACTION_CREATE_DYNAMICALLY_LINKED_EXECUTABLE 4

int main(int argc, char * argv[]) {
    const char * options[7] = { "-shared", "-Bshareable", "-static", "--static", "-pie", "-c", "-o" };
          int    indexes[7] = {    -1,          -1,           -1,       -1,        -1,    -1,   -1  };

    for (int i = 1; i < argc; i++) {
        for (int j = 0; j < 7; j++) {
            if (strcmp(argv[i], options[j]) == 0) {
                indexes[j] = i;
                break;
            }
        }
    }

    int action = 0;

    if (indexes[0] > 0 || indexes[1] > 0) {
        // if -shared option is specified, then remove -static , --static , -pie options if they also are specified
        action = ACTION_CREATE_SHARED_LIBRARY;
    } else if ((indexes[2] > 0) || (indexes[3] > 0)) {
        // if -shared option is not specified, but -static or --static option is specified, then remove -pie , -Wl,-Bdynamic option if it also is specified
        action = ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE;
    } else if (indexes[4] > 0) {
        action = ACTION_CREATE_DYNAMICALLY_LINKED_EXECUTABLE;
    }

    /////////////////////////////////////////////////////////////////

    char * const clang = getenv("PROXIED_CC");

    if (clang == NULL) {
        fprintf(stderr, "PROXIED_CC environment variable is not set.\n");
        return 1;
    }

    if (clang[0] == '\0') {
        fprintf(stderr, "PROXIED_CC environment variable value should be a non-empty string.\n");
        return 2;
    }

    /////////////////////////////////////////////////////////////////

    const char * const TARGET = getenv("CLANG_TARGET");

    if (TARGET == NULL) {
        fprintf(stderr, "CLANG_TARGET environment variable is not set.\n");
        return 3;
    }

    if (TARGET[0] == '\0') {
        fprintf(stderr, "CLANG_TARGET environment variable value should be a non-empty string.\n");
        return 4;
    }

    size_t targetArgLength = strlen(TARGET) + 10U;
    char   targetArg[targetArgLength];

    int ret = snprintf(targetArg, targetArgLength, "--target=%s", TARGET);

    if (ret < 0) {
        perror(NULL);
        return 5;
    }

    /////////////////////////////////////////////////////////////////

    const char * const SYSROOT = getenv("SYSROOT");

    if (SYSROOT == NULL) {
        fprintf(stderr, "SYSROOT environment variable is not set.\n");
        return 6;
    }

    if (SYSROOT[0] == '\0') {
        fprintf(stderr, "SYSROOT environment variable value should be a non-empty string.\n");
        return 7;
    }

    size_t sysrootArgLength = strlen(SYSROOT) + 11U;
    char   sysrootArg[sysrootArgLength];

    ret = snprintf(sysrootArg, sysrootArgLength, "--sysroot=%s", SYSROOT);

    if (ret < 0) {
        perror(NULL);
        return 8;
    }

    /////////////////////////////////////////////////////////////////

    char* argv2[argc + 4];

    argv2[0] = clang;
    argv2[1] = targetArg;
    argv2[2] = sysrootArg;

    if (action == ACTION_CREATE_SHARED_LIBRARY) {
        argv2[3] = (char*)"-fPIC";

        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-static") == 0) {
                argv2[i + 3] = (char*)"-fPIC";
            } else if (strcmp(argv[i], "--static") == 0) {
                argv2[i + 3] = (char*)"-fPIC";
            } else if (strcmp(argv[i], "-pie") == 0) {
                argv2[i + 3] = (char*)"-fPIC";
            } else {
                argv2[i + 3] = argv[i];
            }
        }

        argv2[argc + 3] = NULL;
    } else if (action == ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-rdynamic") == 0) {
                argv2[i + 2] = (char*)"-static";
            } else if (strcmp(argv[i], "-Wl,--export-dynamic") == 0) {
                argv2[i + 2] = (char*)"-static";
            } else if (strcmp(argv[i], "-Wl,-Bdynamic") == 0) {
                argv2[i + 2] = (char*)"-static";
            } else if (strcmp(argv[i], "-pie") == 0) {
                argv2[i + 2] = (char*)"-static";
            } else {
                argv2[i + 2] = argv[i];
            }
        }

        argv2[argc + 2] = NULL;
    } else {
        for (int i = 1; i < argc; i++) {
            argv2[i + 2] = argv[i];
        }

        argv2[argc + 2] = NULL;
    }

    execv (clang, argv2);
    perror(clang);
    return 255;
}
