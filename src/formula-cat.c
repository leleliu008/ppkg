#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

#include "ppkg.h"

int ppkg_formula_cat(const char * packageName) {
    char * formulaFilePath = NULL;

    int ret = ppkg_formula_locate(packageName, &formulaFilePath);

    if (ret != PPKG_OK) {
        return ret;
    }

    int fd = open(formulaFilePath, O_RDONLY);

    if (fd == -1) {
        perror(formulaFilePath);
        free(formulaFilePath);
        return PPKG_ERROR;
    }

    printf("formula: %s\n", formulaFilePath);

    free(formulaFilePath);
    formulaFilePath = NULL;

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
