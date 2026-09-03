#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

int main(int argc, char* argv[]) {
    char * const sed = getenv("SED");

    if (sed == NULL || sed[0] == '\0') {
        fprintf(stderr, "SED environment variable was not set.\n");
        return 1;
    }

    /////////////////////////////////////////////////////////////////

    char* args[argc + 2];

    args[0] = sed;
    args[1] = (char*)"-i";

    for (int i = 1; i < argc; i++) {
        args[i + 1] = argv[i];
    }

    args[argc + 1] = NULL;

    for (int i = 0; args[i] != NULL; i++) {
        fprintf(stderr, "%s ", args[i]);
    }
    fprintf(stderr, "\n");

    execv (sed, args);
    perror(sed);
    return 255;
}
