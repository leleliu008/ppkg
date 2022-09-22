#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char * argv[]) {
    char * PROXIED_PROGRAM = NULL;

    if (argv[0][strlen(argv[0]) - 1] == '+') {
        PROXIED_PROGRAM = getenv("PROXIED_CXX");

        if (PROXIED_PROGRAM == NULL) {
            fprintf(stderr, "PROXIED_CXX environment variable is not set.\n");
            return 2;
        }
    } else {
        PROXIED_PROGRAM = getenv("PROXIED_CC");

        if (PROXIED_PROGRAM == NULL) {
            fprintf(stderr, "PROXIED_CC environment variable is not set.\n");
            return 2;
        }
    }

    if (argc > 1) {
        const char * options[5] = { "-shared", "-static", "--static", "-Wl,-Bdynamic", "-pie" };
              int    indexes[5] = {    -1,         -1,        -1,            -1,         -1   };

        for (int i = 1; i < argc; i++) {
            for (int j = 0; j < 5; j++) {
                if (strcmp(argv[i], options[j]) == 0) {
                    indexes[j] = i;
                }

                if (indexes[0] > 0) {
                    if ((indexes[1] > 0) && (indexes[2] > 0) && (indexes[4] > 0)) {
                        goto lable;
                    }
                }
            }
        }

        lable:
         printf("      -shared = %d\n", indexes[0]);
         printf("      -static = %d\n", indexes[1]);
         printf("     --static = %d\n", indexes[2]);
         printf("-Wl,-Bdynamic = %d\n", indexes[3]);
         printf("         -pie = %d\n", indexes[4]);

        // if -shared option is passed, then remove -static , --static , -pie options if they also are passed
        if (indexes[0] > 0) {
            if (indexes[1] > 0) {
                argv[indexes[1]] = (char*)"-shared";
            }
            if (indexes[2] > 0) {
                argv[indexes[2]] = (char*)"-shared";
            }
            if (indexes[4] > 0) {
                argv[indexes[4]] = (char*)"-shared";
            }
        } else {
            // if -shared option is not passed, but -static or --static option is passed, then remove -pie , -Wl,-Bdynamic option if it also is passed
            if ((indexes[1] > 0) || (indexes[2] > 0)) {
                if (indexes[3] > 0) {
                    argv[indexes[3]] = (char*)"-static";
                }
                if (indexes[4] > 0) {
                    argv[indexes[4]] = (char*)"-static";
                }
            }
        }
    }

    argv[0] = PROXIED_PROGRAM;

     for (int i = 0; argv[i] != NULL; i++) {
         printf("%s ", argv[i]);
     }
     printf("\n");

    if (PROXIED_PROGRAM[0] == '/') {
        if (execv(PROXIED_PROGRAM, argv) == -1) {
            perror(PROXIED_PROGRAM);
            return 1;
        }
    } else {
        if (execvp(PROXIED_PROGRAM, argv) == -1) {
            fprintf(stderr, "command not found: %s\b", PROXIED_PROGRAM);
            return 1;
        }
    }

    return 0;
}
