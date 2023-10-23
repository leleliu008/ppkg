#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "ppkg.h"

int ppkg_session_dir(char buf[], size_t bufSize, size_t * outSize) {
    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    int ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    struct stat st;

    size_t   ppkgRunDIRLength = ppkgHomeDIRLength + 5U;
    char     ppkgRunDIR[ppkgRunDIRLength];
    snprintf(ppkgRunDIR, ppkgRunDIRLength, "%s/run", ppkgHomeDIR);

    if (lstat(ppkgRunDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(ppkgRunDIR) != 0) {
                perror(ppkgRunDIR);
                return PPKG_ERROR;
            }

            if (mkdir(ppkgRunDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(ppkgRunDIR);
                    return PPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(ppkgRunDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(ppkgRunDIR);
                return PPKG_ERROR;
            }
        }
    }

    ////////////////////////////////////////////////////////////////

    size_t   sessionDIRCapacity = ppkgRunDIRLength + 20U;
    char     sessionDIR[sessionDIRCapacity];
    snprintf(sessionDIR, sessionDIRCapacity, "%s/%d", ppkgRunDIR, getpid());

    if (lstat(sessionDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            ret = ppkg_rm_r(sessionDIR, false);

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

    size_t sessionDIRLength = strlen(sessionDIR);

    size_t m = bufSize - 1U;

    size_t n = sessionDIRLength > m ? m : sessionDIRLength;

    strncpy(buf, sessionDIR, n);

    buf[n] = '\0';

    if (outSize != NULL) {
        (*outSize) = n;
    }

    return PPKG_OK;
}
