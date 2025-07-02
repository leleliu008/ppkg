#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#define ACTION_PREPROCESS                           1
#define ACTION_COMPILE                              2
#define ACTION_ASSEMBLE                             3
#define ACTION_CREATE_SHARED_LIBRARY                4
#define ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE  5

int main(int argc, char * argv[]) {
    char * const compiler = getenv("PROXIED_TARGET_CXX");

    if (compiler == NULL) {
        fprintf(stderr, "PROXIED_TARGET_CXX environment variable is not set.\n");
        return 1;
    }

    if (compiler[0] == '\0') {
        fprintf(stderr, "PROXIED_TARGET_CXX environment variable value should be a non-empty string.\n");
        return 2;
    }

    /////////////////////////////////////////////////////////////////

    int action = 0;

    int staticFlag = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            continue;
        }

        if (argv[i][1] == 'c' && argv[i][2] == '\0') {
            action = ACTION_ASSEMBLE;
            break;
        }

        if (argv[i][1] == 'E' && argv[i][2] == '\0') {
            action = ACTION_PREPROCESS;
            break;
        }

        if (argv[i][1] == 'S' && argv[i][2] == '\0') {
            action = ACTION_COMPILE;
            break;
        }

#if defined (__APPLE__)
        if (argv[i][1] == 'd' && strcmp(argv[i], "-dynamiclib") == 0) {
            action = ACTION_CREATE_SHARED_LIBRARY;
            break;
        }

        if (argv[i][1] == 's' && strcmp(argv[i], "-shared") == 0) {
            action = ACTION_CREATE_SHARED_LIBRARY;
            break;
        }
    }
#else
        if (argv[i][1] == 's') {
            if (strcmp(argv[i], "-shared") == 0) {
                action = ACTION_CREATE_SHARED_LIBRARY;
                break;
            }

            if (staticFlag == 1) {
                continue;
            }

            if (strcmp(argv[i], "-static") == 0) {
                staticFlag = 1;
            }

            continue;
        }

        if (staticFlag == 1) {
            continue;
        }

        if (argv[i][1] == '-' && strcmp(argv[i], "--static") == 0) {
            staticFlag = 1;
        }
    }

    if (action == 0) {
        if (staticFlag == 1) {
            action = ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE;
        }
    }
#endif

    //fprintf(stderr, "action=%d\n", action);

    /////////////////////////////////////////////////////////////////

    int n = 0;

    char * const baseArgs = getenv("WRAPPER_TARGET_CXXFLAGS");

    if (baseArgs != NULL) {
        char * p = baseArgs;

        while (p[0] != '\0') {
            if (p[0] == ' ') {
                p++;
                continue;
            }

            n++;

            for (;;) {
                p++;

                if (p[0] == '\0') {
                    break;
                }

                if (p[0] == ' ') {
                    p++;
                    break;
                }
            }
        }
    }

    //fprintf(stderr, "n=%d\n", n);

    /////////////////////////////////////////////////////////////////

    int m = 0;

    char * ldflags = NULL;

    if (action == 0 || action == ACTION_CREATE_SHARED_LIBRARY || action == ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE) {
        if ((ldflags = getenv("WRAPPER_TARGET_LDFLAGS")) != NULL) {
            char * p = ldflags;

            while (p[0] != '\0') {
                if (p[0] == ' ') {
                    p++;
                    continue;
                }

                m++;

                for (;;) {
                    p++;

                    if (p[0] == '\0') {
                        break;
                    }

                    if (p[0] == ' ') {
                        p++;
                        break;
                    }
                }
            }
        }

        //fprintf(stderr, "m=%d\n", m);
    }

    /////////////////////////////////////////////////////////////////

    char* args[argc + n + m + 5];

    if (action == ACTION_PREPROCESS || action == ACTION_COMPILE || action == ACTION_ASSEMBLE) {
        for (int i = 1; i < argc; i++) {
            args[i] = argv[i];
        }
    } else if (action == ACTION_CREATE_SHARED_LIBRARY) {
        // remove -static , --static , -pie options if they also are specified
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-static") == 0) {
                args[i] = (char*)"-fPIC";
            } else if (strcmp(argv[i], "--static") == 0) {
                args[i] = (char*)"-fPIC";
            } else if (strcmp(argv[i], "-pie") == 0) {
                args[i] = (char*)"-fPIC";
            } else {
                args[i] = argv[i];
            }
        }
    } else if (action == ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE) {
        // remove -pie , -Wl,-Bdynamic option if it also is specified
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-rdynamic") == 0) {
                args[i] = (char*)"-static";
            } else if (strcmp(argv[i], "-Wl,--export-dynamic") == 0) {
                args[i] = (char*)"-static";
            } else if (strcmp(argv[i], "-Wl,-Bdynamic") == 0) {
                args[i] = (char*)"-static";
            } else if (strcmp(argv[i], "-pie") == 0) {
                args[i] = (char*)"-static";
            } else if (argv[i][0] == '/') {
                int nulIndex = 0;
                int slashIndex = 0;

                for (int j = 1; ; j++) {
                    if (argv[i][j] == '\0') {
                        nulIndex = j;
                        break;
                    } else if (argv[i][j] == '/') {
                        slashIndex = j;
                    }
                }

                char * filename = argv[i] + slashIndex + 1;

                fprintf(stderr, "filename=%s\n", filename);

                if ((argv[i][nulIndex - 3] == '.') && (argv[i][nulIndex - 2] == 's') && (argv[i][nulIndex - 1] == 'o')) {
                    if (strcmp(filename, "libm.so") == 0) {
                        argv[i][0] = '-';
                        argv[i][1] = 'l';
                        argv[i][2] = 'm';
                        argv[i][3] = '\0';
                    } else if (strcmp(filename, "libdl.so") == 0) {
                        argv[i][0] = '-';
                        argv[i][1] = 'l';
                        argv[i][2] = 'd';
                        argv[i][3] = 'l';
                        argv[i][4] = '\0';
                    } else {
                        argv[i][nulIndex - 2] = 'a';
                        argv[i][nulIndex - 1] = '\0';

                        struct stat st;

                        if (stat(argv[i], &st) != 0 || !S_ISREG(st.st_mode)) {
                            argv[i][nulIndex - 2] = 's';
                            argv[i][nulIndex - 1] = 'o';
                        }
                    }
                } else {
                    char * p = strstr(filename, ".so");

                    if (p != NULL) {
                        p[1] = 'a' ;
                        p[2] = '\0';

                        struct stat st;

                        if (stat(argv[i], &st) != 0 || !S_ISREG(st.st_mode)) {
                            p[1] = 's' ;
                            p[2] = 'o';
                        }
                    }
                }

                args[i] = argv[i];
            } else {
                args[i] = argv[i];
            }
        }
    } else {
        const char * const msle = getenv("PACKAGE_CREATE_MOSTLY_STATICALLY_LINKED_EXECUTABLE");

        if (msle != NULL && msle[0] == '1' && msle[1] == '\0') {
            for (int i = 1; i < argc; i++) {
                if (argv[i][0] == '/') {
                    int nulIndex = 0;
                    int dotIndex = -1;
                    int slashIndex = 0;

                    for (int j = 1; ; j++) {
                        if (argv[i][j] == '\0') {
                            nulIndex = j;
                            break;
                        }

                        if (argv[i][j] == '.') {
                            dotIndex = j;
                        } else if (argv[i][j] == '/') {
                            slashIndex = j;
                        }
                    }

                    if (dotIndex == -1) {
                        args[i] = argv[i];
                        continue;
                    }

#if defined (__APPLE__)
                    if (strcmp(&argv[i][dotIndex], ".dylib") == 0) {
                        argv[i][dotIndex + 1] = 'a' ;
                        argv[i][dotIndex + 2] = '\0';

                        struct stat st;

                        if (stat(argv[i], &st) != 0 || !S_ISREG(st.st_mode)) {
                            argv[i][dotIndex + 1] = 'd';
                            argv[i][dotIndex + 2] = 'y';
                        }
                    }
#else
                    if (argv[i][dotIndex + 1] == 'a' && argv[i][dotIndex + 2] == '\0') {
                        if (argv[i][slashIndex + 1] == 'l' && argv[i][slashIndex + 2] == 'i' && argv[i][slashIndex + 3] == 'b') {
                            if (argv[i][slashIndex + 4] == 'm') {
                                int filenameLen = nulIndex - slashIndex - 1;
                                if (filenameLen == 6) {
                                    argv[i][0] = '-';
                                    argv[i][1] = 'l';
                                    argv[i][2] = 'm';
                                    argv[i][3] = '\0';
                                } else {
                                    // /usr/lib/x86_64-linux-gnu/libm-2.39.a
                                    if ((filenameLen == 11) && (argv[i][slashIndex + 5] == '-') && (argv[i][slashIndex + 6] == '2') && (argv[i][slashIndex + 7] == '.') && (argv[i][slashIndex + 8] > '0') && (argv[i][slashIndex + 8] <= '9') && (argv[i][slashIndex + 9] > '0') && (argv[i][slashIndex + 9] <= '9')) {
                                        argv[i][0] = '-';
                                        argv[i][1] = 'l';
                                        argv[i][2] = 'm';
                                        argv[i][3] = '\0';
                                    }
                                }
                            }
                        }
                    } else if (argv[i][dotIndex + 1] == 's' && argv[i][dotIndex + 2] == 'o' && argv[i][dotIndex + 3] == '\0') {
                        argv[i][dotIndex + 1] = 'a' ;
                        argv[i][dotIndex + 2] = '\0';

                        struct stat st;

                        if (stat(argv[i], &st) != 0 || !S_ISREG(st.st_mode)) {
                            argv[i][dotIndex + 1] = 's';
                            argv[i][dotIndex + 2] = 'o';
                        }
                    } else {
                        char * p = strstr(&argv[i][slashIndex + 1], ".so");

                        if (p != NULL) {
                            p[1] = 'a' ;
                            p[2] = '\0';

                            struct stat st;

                            if (stat(argv[i], &st) != 0 || !S_ISREG(st.st_mode)) {
                                p[1] = 's' ;
                                p[2] = 'o';
                            }
                        }
                    }
#endif
                }

                args[i] = argv[i];
            }
        } else {
            for (int i = 1; i < argc; i++) {
                args[i] = argv[i];
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    if (n != 0) {
        char * p = baseArgs;

        while (p[0] != '\0') {
            if (p[0] == ' ') {
                p++;
                continue;
            }

            args[argc++] = p;

            for (;;) {
                p++;

                if (p[0] == '\0') {
                    break;
                }

                if (p[0] == ' ') {
                    p[0] = '\0';
                    p++;
                    break;
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    if (m != 0) {
        char * p = ldflags;

        while (p[0] != '\0') {
            if (p[0] == ' ') {
                p++;
                continue;
            }

            args[argc++] = p;

            for (;;) {
                p++;

                if (p[0] == '\0') {
                    break;
                }

                if (p[0] == ' ') {
                    p[0] = '\0';
                    p++;
                    break;
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    if (action == ACTION_ASSEMBLE || action == ACTION_CREATE_SHARED_LIBRARY) {
        args[argc++] = (char*)"-fPIC";
    }

    args[argc++] = NULL;
    args[0] = compiler;

    /////////////////////////////////////////////////////////////////

    const char * const verbose = getenv("PPKG_VERBOSE");

    if (verbose != NULL && verbose[0] == '1' && verbose[1] == '\0') {
        for (int i = 0; args[i] != NULL; i++) {
            fprintf(stderr, "%s\n", args[i]);
        }
    }

    /////////////////////////////////////////////////////////////////

    execv (compiler, args);
    perror(compiler);
    return 255;
}
