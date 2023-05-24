#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "ppkg.h"

int ppkg_home_dir(char buf[], size_t bufSize, size_t * outSize) {
    if (buf == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (bufSize == 0U) {
        return PPKG_ERROR_ARG_IS_INVALID;
    }

    const char * const ppkgHomeDir = getenv("PPKG_HOME");

    if (ppkgHomeDir == NULL) {
        const char * const userHomeDir = getenv("HOME");

        if (userHomeDir == NULL) {
            return PPKG_ERROR_ENV_HOME_NOT_SET;
        }

        size_t userHomeDirLength = strlen(userHomeDir);

        if (userHomeDirLength == 0U) {
            return PPKG_ERROR_ENV_HOME_NOT_SET;
        }

        size_t   ppkgHomeDirLength = userHomeDirLength + + 7U;
        char     ppkgHomeDir[ppkgHomeDirLength];
        snprintf(ppkgHomeDir, ppkgHomeDirLength, "%s/.ppkg", userHomeDir);

        struct stat st;

        if (stat(ppkgHomeDir, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", ppkgHomeDir);
                return PPKG_ERROR;
            }
        } else {
            if (mkdir(ppkgHomeDir, S_IRWXU) != 0) {
                perror(ppkgHomeDir);
                return PPKG_ERROR;
            }
        }

        size_t n = (bufSize > ppkgHomeDirLength) ? ppkgHomeDirLength : bufSize;

        strncpy(buf, ppkgHomeDir, n);

        if (outSize != NULL) {
            (*outSize) = n;
        }
    } else {
        if (ppkgHomeDir[0] == '\0') {
            fprintf(stderr, "'PPKG_HOME' environment variable's value was expected to be a non-empty string, but it was not.\n");
            return PPKG_ERROR;
        }

        struct stat st;

        if (stat(ppkgHomeDir, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", ppkgHomeDir);
                return PPKG_ERROR;
            }
        } else {
            if (mkdir(ppkgHomeDir, S_IRWXU) != 0) {
                perror(ppkgHomeDir);
                return PPKG_ERROR;
            }
        }

        size_t ppkgHomeDirLength = strlen(ppkgHomeDir);

        size_t n = (bufSize > ppkgHomeDirLength) ? ppkgHomeDirLength : bufSize;

        strncpy(buf, ppkgHomeDir, n);

        if (outSize != NULL) {
            (*outSize) = n;
        }
    }

    return PPKG_OK;
}
