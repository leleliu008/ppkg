#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "rm-r.h"

int rm_r(const char * dirPath, bool verbose) {
    if (dirPath == NULL) {
        errno = EINVAL;
        return -1;
    }

    size_t dirPathLength = strlen(dirPath);

    if (dirPathLength == 0U) {
        errno = EINVAL;
        return -1;
    }

    DIR * dir = opendir(dirPath);

    if (dir == NULL) {
        return -1;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);

                return rmdir(dirPath);
            } else {
                int err = errno;
                closedir(dir);
                errno = err;
                return -1;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t   filePathLength = dirPathLength + strlen(dir_entry->d_name) + 2U;
        char     filePath[filePathLength];
        snprintf(filePath, filePathLength, "%s/%s", dirPath, dir_entry->d_name);

        if (verbose) printf("rm %s\n", filePath);

        struct stat st;

        if (stat(filePath, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                if (rm_r(filePath, verbose) != 0) {
                    int err = errno;
                    closedir(dir);
                    errno = err;
                    return -1;
                }
            } else {
                if (unlink(filePath) != 0) {
                    int err = errno;
                    closedir(dir);
                    errno = err;
                    return -1;
                }
            }
        } else {
            // why does this happened?
            // Suppose you have following file structure:
            // bin
            // ├── gsed
            // └── sed -> gsed
            // if bin/gsed was removed, then bin/sed will be treated as a non-existent file.

            if (unlink(filePath) != 0) {
                int err = errno;
                closedir(dir);
                errno = err;
                return -1;
            }
        }
    }
}
