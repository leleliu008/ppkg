#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char * argv[]) {
    char * WRAPPED_PROGRAM = NULL;

    if (argv[0][strlen(argv[0]) - 1] == '+') {
        WRAPPED_PROGRAM = getenv("WRAPPED_CXX");

        if (WRAPPED_PROGRAM == NULL) {
            fprintf(stderr, "WRAPPED_CXX environment variable is not set.\n");
            return 2;
        }
    } else {
        WRAPPED_PROGRAM = getenv("WRAPPED_CC");

        if (WRAPPED_PROGRAM == NULL) {
            fprintf(stderr, "WRAPPED_CC environment variable is not set.\n");
            return 2;
        }
    }

    if (argc > 1) {
        int sharedOptionIndex = -1;
        int staticOptionIndex = -1;
        int static2OptionIndex = -1;

        int lastIndex = argc - 1;

        for (int i = 0; i <= lastIndex; i++) {
            if (strcmp(argv[i], "-shared") == 0) {
                sharedOptionIndex = i;

                if ((staticOptionIndex > 0) && (static2OptionIndex > 0)) {
                    break;
                }
            }

            if (strcmp(argv[i], "-static") == 0) {
                staticOptionIndex = i;

                if ((sharedOptionIndex > 0) && (static2OptionIndex > 0)) {
                    break;
                }
            }

            if (strcmp(argv[i], "--static") == 0) {
                static2OptionIndex = i;

                if ((sharedOptionIndex > 0) && (staticOptionIndex > 0)) {
                    break;
                }
            }
        }

        //printf("sharedOptionIndex=%d\n", sharedOptionIndex);
        //printf("staticOptionIndex=%d\n", staticOptionIndex);
        //printf("static2OptionIndex=%d\n", static2OptionIndex);

        if (sharedOptionIndex > 0) {
            if (staticOptionIndex > 0) {
                for (int i = staticOptionIndex; i < lastIndex; i++) {
                    argv[i] = argv[i + 1];
                }

                argv[lastIndex] = NULL;
                lastIndex -= 1;

                if (static2OptionIndex > staticOptionIndex) {
                    static2OptionIndex -= 1;
                }
            }

            if (static2OptionIndex > 0) {
                for (int i = static2OptionIndex; i < lastIndex; i++) {
                    argv[i] = argv[i + 1];
                }
                argv[lastIndex] = NULL;
            }
        }
    }

    argv[0] = WRAPPED_PROGRAM;

    if (WRAPPED_PROGRAM[0] == '/') {
        if (execv(WRAPPED_PROGRAM, argv) == -1) {
            perror(WRAPPED_PROGRAM);
            return 1;
        }
    } else {
        if (execvp(WRAPPED_PROGRAM, argv) == -1) {
            fprintf(stderr, "command not found: %s\b", WRAPPED_PROGRAM);
            return 1;
        }
    }

    return 0;
}
