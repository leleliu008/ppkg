#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

#include "ppkg.h"

int ppkg_formula_cat(const char * packageName, const char * targetPlatformName) {
    char formulaFilePath[PATH_MAX];

    int ret = ppkg_formula_path(packageName, targetPlatformName, formulaFilePath);

    if (ret != PPKG_OK) {
        return ret;
    }

    int fd = open(formulaFilePath, O_RDONLY);

    if (fd == -1) {
        perror(formulaFilePath);
        return PPKG_ERROR;
    }

    printf("formula: %s\n", formulaFilePath);

    char buf[1024];

    for(;;) {
        ssize_t readSize = read(fd, buf, 1024);

        if (readSize == -1) {
            perror(formulaFilePath);
            close(fd);
            return PPKG_ERROR;
        }

        if (readSize == 0) {
            close(fd);
            return PPKG_OK;
        }

        ssize_t writeSize = write(STDOUT_FILENO, buf, readSize);

        if (writeSize == -1) {
            perror(NULL);
            close(fd);
            return PPKG_ERROR;
        }

        if (writeSize != readSize) {
            fprintf(stderr, "not fully written to stdout.");
            close(fd);
            return PPKG_ERROR;
        }
    }
}
