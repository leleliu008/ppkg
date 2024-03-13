#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "ppkg.h"

int ppkg_rename_or_copy_file(const char * fromFilePath, const char * toFilePath) {
    if (fromFilePath == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (toFilePath == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (fromFilePath[0] == '\0') {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (toFilePath[0] == '\0') {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    size_t i = 0U;
    size_t j = 0U;

    for (;;) {
        char c = toFilePath[i];

        if (c == '\0') {
            break;
        }

        if (c == '/') {
            j = i;
        }

        i++;
    }

    if (j > 0U) {
        char outputDIR[j + 2U];

        strncpy(outputDIR, toFilePath, j);

        outputDIR[j] = '\0';

        int ret = ppkg_mkdir_p(outputDIR, false);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    if (rename(fromFilePath, toFilePath) == 0) {
        return PPKG_OK;
    } else {
        if (errno == EXDEV) {
            return ppkg_copy_file(fromFilePath, toFilePath);
        } else {
            perror(toFilePath);
            return PPKG_ERROR;
        }
    }
}
