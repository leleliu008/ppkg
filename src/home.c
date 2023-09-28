#include <errno.h>
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

    const char * const ppkgHomeDIR = getenv("PPKG_HOME");

    if (ppkgHomeDIR == NULL) {
        const char * const userHomeDIR = getenv("HOME");

        if (userHomeDIR == NULL) {
            return PPKG_ERROR_ENV_HOME_NOT_SET;
        }

        if (userHomeDIR[0] == '\0') {
            return PPKG_ERROR_ENV_HOME_NOT_SET;
        }

        size_t   defaultUppmHomeDIRLength = strlen(userHomeDIR) + 6U;
        char     defaultUppmHomeDIR[defaultUppmHomeDIRLength + 1U];
        snprintf(defaultUppmHomeDIR, defaultUppmHomeDIRLength + 1U, "%s/.ppkg", userHomeDIR);

        struct stat st;

        if (stat(defaultUppmHomeDIR, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "%s was expected to be a directory, but it was not.\n", defaultUppmHomeDIR);
                return PPKG_ERROR;
            }
        } else {
            if (mkdir(defaultUppmHomeDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(defaultUppmHomeDIR);
                    return PPKG_ERROR;
                }
            }
        }

        size_t m = bufSize - 1U;

        size_t n = (m > defaultUppmHomeDIRLength) ? defaultUppmHomeDIRLength : m;

        strncpy(buf, defaultUppmHomeDIR, n);

        buf[n] = '\0';

        if (outSize != NULL) {
            (*outSize) = n;
        }
    } else {
        if (ppkgHomeDIR[0] == '\0') {
            fprintf(stderr, "'PPKG_HOME' environment variable's value was expected to be a non-empty string, but it was not.\n");
            return PPKG_ERROR;
        }

        struct stat st;

        if (stat(ppkgHomeDIR, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "%s was expected to be a directory, but it was not.\n", ppkgHomeDIR);
                return PPKG_ERROR;
            }
        } else {
            if (mkdir(ppkgHomeDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(ppkgHomeDIR);
                    return PPKG_ERROR;
                }
            }
        }

        size_t ppkgHomeDIRLength = strlen(ppkgHomeDIR);

        size_t m = bufSize - 1U;

        size_t n = (m > ppkgHomeDIRLength) ? ppkgHomeDIRLength : m;

        strncpy(buf, ppkgHomeDIR, n);

        buf[n] = '\0';

        if (outSize != NULL) {
            (*outSize) = n;
        }
    }

    return PPKG_OK;
}
