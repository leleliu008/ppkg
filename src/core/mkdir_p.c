#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#include "mkdir_p.h"

int mkdir_p(const char * dirPath, bool verbose) {
    if (dirPath == NULL) {
        errno = EINVAL;
        return -1;
    }

    size_t dirPathLength = strlen(dirPath);

    if (dirPathLength == 0U) {
        errno = EINVAL;
        return -1;
    }

    if (verbose) printf("mkdir -p %s\n", dirPath);

    struct stat st;

    if (stat(dirPath, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return 0;
        } else {
            errno = ENOTDIR;
            return -1;
        }
    } else {
        size_t i = dirPathLength - 1U;

        if (dirPath[i] == '/') {
            i--;
        }

        for(;;) {
            if (dirPath[i] == '/') {
                if (i == 0U) { // /a
                    return mkdir(dirPath, S_IRWXU);
                } else {
                    char p[i];
                    strncpy(p, dirPath, i - 1U);

                    return mkdir_p(p, verbose);
                }
            }

            i--;

            if (i == 0U) {
                // dirPath is a relative path
                return mkdir(dirPath, S_IRWXU);
            }
        }
    }

    return 0;
}
