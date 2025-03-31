#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "ppkg.h"

int ppkg_session_dir(char buf[], size_t * len) {
    char   tmpBuf[PATH_MAX];
    size_t tmpBufLength;

    int ret = ppkg_home_dir(tmpBuf, &tmpBufLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    const char * const str = "/run";

    char * p = tmpBuf + tmpBufLength;

    for (int i = 0; ; i++) {
        p[i] = str[i];

        if (str[i] == '\0') {
            tmpBufLength += i;
            break;
        }
    }

    ////////////////////////////////////////////////////////////////

    struct stat st;

    if (lstat(tmpBuf, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(tmpBuf) != 0) {
                perror(tmpBuf);
                return PPKG_ERROR;
            }

            if (mkdir(tmpBuf, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(tmpBuf);
                    return PPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(tmpBuf, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(tmpBuf);
                return PPKG_ERROR;
            }
        }
    }

    ////////////////////////////////////////////////////////////////

    char sessionDIR[PATH_MAX];

    int n = snprintf(sessionDIR, PATH_MAX, "%s/%d", tmpBuf, getpid());

    if (n < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (lstat(sessionDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            ret = ppkg_rm_rf(sessionDIR, false, false);

            if (ret != PPKG_OK) {
                return ret;
            }

            if (mkdir(sessionDIR, S_IRWXU) != 0) {
                perror(sessionDIR);
                return PPKG_ERROR;
            }
        } else {
            if (unlink(sessionDIR) != 0) {
                perror(sessionDIR);
                return PPKG_ERROR;
            }

            if (mkdir(sessionDIR, S_IRWXU) != 0) {
                perror(sessionDIR);
                return PPKG_ERROR;
            }
        }
    } else {
        if (mkdir(sessionDIR, S_IRWXU) != 0) {
            perror(sessionDIR);
            return PPKG_ERROR;
        }
    }

    strncpy(buf, sessionDIR, n);

    buf[n] = '\0';

    if (len != NULL) {
        (*len) = n;
    }

    return PPKG_OK;
}
