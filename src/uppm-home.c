#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "uppm.h"
#include "ppkg.h"

int uppm_home_dir(char buf[], size_t bufSize, size_t * outSize) {
    if (buf == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (bufSize == 0U) {
        return PPKG_ERROR_ARG_IS_INVALID;
    }

    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength = 0U;

    int ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    ret = snprintf(buf, bufSize, "%s/uppm", ppkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (outSize != NULL) {
        (*outSize) = ret;
    }

    struct stat st;

    if (stat(buf, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "%s was expected to be a directory, but it was not.\n", buf);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(buf, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(buf);
                return PPKG_ERROR;
            }
        }
    }

    return PPKG_OK;
}
