#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/syslimits.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "ppkg.h"

int ppkg_home_dir(char buf[], size_t * len) {
    if (buf == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
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

        char   tmpBuf[PATH_MAX];
        size_t tmpBufLength;

        for (int i = 0; ; i++) {
            if (userHomeDIR[i] == '\0') {
                tmpBufLength = i;
                break;
            } else {
                tmpBuf[i] = userHomeDIR[i];
            }
        }

        char * p = tmpBuf + tmpBufLength;

        const char * const str = "/.ppkg";

        for (int i = 0; ; i++) {
            p[i] = str[i];

            if (str[i] == '\0') {
                tmpBufLength += i;
                break;
            }
        }

        struct stat st;

        if (stat(tmpBuf, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "%s was expected to be a directory, but it was not.\n", tmpBuf);
                return PPKG_ERROR;
            }
        } else {
            if (mkdir(tmpBuf, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(tmpBuf);
                    return PPKG_ERROR;
                }
            }
        }

        strncpy(buf, tmpBuf, tmpBufLength);

        buf[tmpBufLength] = '\0';

        if (len != NULL) {
            (*len) = tmpBufLength;
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

        size_t n = strlen(ppkgHomeDIR);

        strncpy(buf, ppkgHomeDIR, n);

        buf[n] = '\0';

        if (len != NULL) {
            (*len) = n;
        }
    }

    return PPKG_OK;
}
