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
    char * const compiler = getenv("PROXIED_CXX");

    if (compiler == NULL) {
        fprintf(stderr, "PROXIED_CXX environment variable is not set.\n");
        return 1;
    }

    if (compiler[0] == '\0') {
        fprintf(stderr, "PROXIED_CXX environment variable value should be a non-empty string.\n");
        return 2;
    }

    /////////////////////////////////////////////////////////////////

    int action = 0;

    int staticFlag = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-E") == 0) {
            action = ACTION_PREPROCESS;
            break;
        }

        if (strcmp(argv[i], "-S") == 0) {
            action = ACTION_COMPILE;
            break;
        }

        if (strcmp(argv[i], "-c") == 0) {
            action = ACTION_ASSEMBLE;
            break;
        }

#if defined (__APPLE__)
        if (strcmp(argv[i], "-dynamiclib") == 0) {
            action = ACTION_CREATE_SHARED_LIBRARY;
            break;
        }
#endif

        if (strcmp(argv[i], "-shared") == 0) {
            action = ACTION_CREATE_SHARED_LIBRARY;
            break;
        }

        if (staticFlag == 1) {
            continue;
        }

        if (strcmp(argv[i], "-static") == 0 || strcmp(argv[i], "--static") == 0) {
            staticFlag = 1;
        }
    }

    if (action == 0) {
        if (staticFlag == 1) {
            action = ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE;
        }
    }

    /////////////////////////////////////////////////////////////////

    int baseArgc;

    char * const baseArgs = getenv("PROXIED_CXX_ARGS");

    if (baseArgs == NULL || baseArgs[0] == '\0') {
        baseArgc = 0;
    } else {
        baseArgc = 1;

        for (int i = 0; ; i++) {
            if (baseArgs[i] == '\0') {
                break;
            }

            if (baseArgs[i] == ' ') {
                baseArgc++;
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    char* argv2[argc + baseArgc + 5];

    if (action == ACTION_PREPROCESS || action == ACTION_COMPILE || action == ACTION_ASSEMBLE) {
        for (int i = 1; i < argc; i++) {
            argv2[i] = argv[i];
        }
    } else if (action == ACTION_CREATE_SHARED_LIBRARY) {
        // remove -static , --static , -pie options if they also are specified
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-static") == 0) {
                argv2[i] = (char*)"-fPIC";
            } else if (strcmp(argv[i], "--static") == 0) {
                argv2[i] = (char*)"-fPIC";
            } else if (strcmp(argv[i], "-pie") == 0) {
                argv2[i] = (char*)"-fPIC";
            } else {
                argv2[i] = argv[i];
            }
        }
    } else if (action == ACTION_CREATE_STATICALLY_LINKED_EXECUTABLE) {
        // remove -pie , -Wl,-Bdynamic option if it also is specified
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-rdynamic") == 0) {
                argv2[i] = (char*)"-static";
            } else if (strcmp(argv[i], "-Wl,--export-dynamic") == 0) {
                argv2[i] = (char*)"-static";
            } else if (strcmp(argv[i], "-Wl,-Bdynamic") == 0) {
                argv2[i] = (char*)"-static";
            } else if (strcmp(argv[i], "-pie") == 0) {
                argv2[i] = (char*)"-static";
            } else if (argv[i][0] == '/') {
                int len = 0;

                int slashIndex = -1;

                for (int j = 0; ; j++) {
                    if (argv[i][j] == '\0') {
                        len = j;
                        break;
                    } else if (argv[i][j] == '/') {
                        slashIndex = j;
                    }
                }

                const char * filename = argv[i] + slashIndex + 1;

                fprintf(stderr, "filename=%s\n", filename);

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
                    if ((argv[i][len - 3] == '.') && (argv[i][len - 2] == 's') && (argv[i][len - 1] == 'o')) {
                        argv[i][len - 2] = 'a';
                        argv[i][len - 1] = '\0';

                        struct stat st;

                        if (stat(argv[i], &st) != 0 || !S_ISREG(st.st_mode)) {
                            argv[i][len - 2] = 's';
                            argv[i][len - 1] = 'o';
                        }
                    }
                }

                argv2[i] = argv[i];
            } else {
                argv2[i] = argv[i];
            }
        }
    } else {
        const char * msle = getenv("PACKAGE_CREATE_MOSTLY_STATICALLY_LINKED_EXECUTABLE");

        if (msle != NULL && strcmp(msle, "1") == 0) {
            for (int i = 1; i < argc; i++) {
                if (argv[i][0] == '/') {
                    int len = 0;
                    int dotIndex = -1;

                    for (int j = 0; ; j++) {
                        if (argv[i][j] == '\0') {
                            len = j;
                            break;
                        }

                        if (argv[i][j] == '.') {
                            dotIndex = j;
                        }
                    }

                    if (dotIndex > 0) {
#if defined (__APPLE__)
                        if (len - dotIndex == 6) {
                            if (strcmp(&argv[i][dotIndex], ".dylib") == 0) {
                                argv[i][dotIndex + 1] = 'a' ;
                                argv[i][dotIndex + 2] = '\0';

                                struct stat st;

                                if (stat(argv[i], &st) != 0 || !S_ISREG(st.st_mode)) {
                                    argv[i][dotIndex + 1] = 'd';
                                    argv[i][dotIndex + 2] = 'y';
                                }
                            }
                        }
#else
                        if (len - dotIndex == 3) {
                            if (strcmp(&argv[i][dotIndex], ".so") == 0) {
                                argv[i][dotIndex + 1] = 'a' ;
                                argv[i][dotIndex + 2] = '\0';

                                struct stat st;

                                if (stat(argv[i], &st) != 0 || !S_ISREG(st.st_mode)) {
                                    argv[i][dotIndex + 1] = 's';
                                    argv[i][dotIndex + 2] = 'o';
                                }
                            }
                        }
#endif
                    }
                }

                argv2[i] = argv[i];
            }
        } else {
            for (int i = 1; i < argc; i++) {
                argv2[i] = argv[i];
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    if (baseArgc != 0) {
        char * p = baseArgs;

        for (int i = 0; ; i++) {
            if (baseArgs[i] == '\0') {
                if (p[0] != '\0') {
                    argv2[argc++] = p;
                }
                break;
            }

            if (baseArgs[i] == ' ') {
                baseArgs[i] = '\0';

                if (p[0] != '\0') {
                    argv2[argc++] = p;
                }

                p = &baseArgs[i + 1];
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    if (action == ACTION_ASSEMBLE || action == ACTION_CREATE_SHARED_LIBRARY) {
        argv2[argc++] = (char*)"-fPIC";
    }

    argv2[argc++] = NULL;
    argv2[0] = compiler;

    /////////////////////////////////////////////////////////////////

    const char * verbose = getenv("PPKG_VERBOSE");

    if (verbose != NULL && strcmp(verbose, "1") == 0) {
        for (int i = 0; ;i++) {
            if (argv2[i] == NULL) {
                break;
            } else {
                fprintf(stderr, "%s\n", argv2[i]);
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    execv (compiler, argv2);
    perror(compiler);
    return 255;
}
