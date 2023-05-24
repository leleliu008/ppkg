#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

#include "rm-r.h"

int rm_r(const char * dirPath, bool verbose) {
    if (dirPath == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    size_t dirPathLength = strlen(dirPath);

    if (dirPathLength == 0) {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    DIR * dir = opendir(dirPath);

    if (dir == NULL) {
        perror(dirPath);
        return PPKG_ERROR;
    }

    int ret = PPKG_OK;

    struct stat st;

    struct dirent * dir_entry;

    for (;;) {
        errno = 0;

        dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);

                if (rmdir(dirPath) == 0) {
                    break;
                } else {
                    perror(dirPath);
                    return PPKG_ERROR;
                }
            } else {
                perror(dirPath);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t filePathLength = dirPathLength + strlen(dir_entry->d_name) + 2;
        char   filePath[filePathLength];
        snprintf(filePath, filePathLength, "%s/%s", dirPath, dir_entry->d_name);

        if (verbose) printf("rm %s\n", filePath);

        if (stat(filePath, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                ret = rm_r(filePath, verbose);

                if (ret != PPKG_OK) {
                    closedir(dir);
                    return PPKG_ERROR;
                }
            } else {
                if (unlink(filePath) != 0) {
                    perror(filePath);
                    closedir(dir);
                    return PPKG_ERROR;
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
                perror(filePath);
                closedir(dir);
                return PPKG_ERROR;
            }
        }
    }

    return PPKG_OK;
}
