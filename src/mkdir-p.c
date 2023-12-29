#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>

#include "ppkg.h"

int ppkg_mkdir_p(const char * dir, const bool verbose) {
    if (dir == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (dir[0] == '\0') {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    size_t len = strlen(dir);

    char buf[len + 1U];

    memset(buf, 0, len + 1U);

    struct stat st;

    int i = 0;

    for (;;) {
        char c = dir[i];

        if (c == '/' || c == '\0') {
            if (buf[0] != '\0') {
                if (stat(buf, &st) == 0) {
                    if (!S_ISDIR(st.st_mode)) {
                        fprintf(stderr, "%s was expected to be a directory, but it was not.\n", buf);
                        return PPKG_ERROR;
                    }
                } else {
                    if (verbose) printf("mkdir -p %s\n", buf);

                    if (mkdir(buf, S_IRWXU) != 0) {
                        if (errno == EEXIST) {
                            if (stat(buf, &st) == 0) {
                                if (!S_ISDIR(st.st_mode)) {
                                    fprintf(stderr, "%s was expected to be a directory, but it was not.\n", buf);
                                    return PPKG_ERROR;
                                }
                            }
                        } else {
                            perror(buf);
                            return PPKG_ERROR;
                        }
                    }
                }
            }
        }

        if (c == '\0') {
            return 0;
        } else {
            buf[i] = c;
            i++;
        }
    }
}
