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

    if (dirPath[0] == '\0') {
        errno = EINVAL;
        return -1;
    }

    size_t len = strlen(dirPath);

    char buf[len + 1U];

    memset(buf, 0, len + 1U);

    struct stat st;

    int i = 0;

    for (;;) {
        char c = dirPath[i];

        if (c == '/' || c == '\0') {
            if (buf[0] != '\0') {
                if (stat(buf, &st) == 0) {
                    if (!S_ISDIR(st.st_mode)) {
                        errno = ENOTDIR;
                        return -1;
                    }
                } else {
                    if (verbose) printf("mkdir -p %s\n", buf);

                    if (mkdir(buf, S_IRWXU) != 0) {
                        return -1;
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
