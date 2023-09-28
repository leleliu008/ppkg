#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>

#include "core/log.h"

#include "ppkg.h"

int ppkg_generate_url_transform_sample() {
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

    ////////////////////////////////////////////////////////////////////////////////////////////

    size_t   sessionDIRLength = ppkgRunDIRLength + 20U;
    char     sessionDIR[sessionDIRLength];
    snprintf(sessionDIR, sessionDIRLength, "%s/%d", ppkgRunDIR, getpid());

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

    ////////////////////////////////////////////////////////////////////////////////////////////

    size_t   tmpFilePathLength = sessionDIRLength + 22U;
    char     tmpFilePath[tmpFilePathLength];
    snprintf(tmpFilePath, tmpFilePathLength, "%s/url-transform.sample", sessionDIR);

    int fd = open(tmpFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(tmpFilePath);
        return PPKG_ERROR;
    }

    const char * p = ""
        "#!/bin/sh\n"
        "case $1 in\n"
        "    *githubusercontent.com/*)\n"
        "        printf 'https://ghproxy.com/%s\\n' \"$1\"\n"
        "        ;;\n"
        "    https://github.com/*)\n"
        "        printf 'https://ghproxy.com/%s\\n' \"$1\"\n"
        "        ;;\n"
        "    '') printf '%s\\n' \"$0 <URL>, <URL> is unspecified.\" >&2 ; exit 1 ;;\n"
        "    *)  printf '%s\\n' \"$1\"\n"
        "esac";

    size_t pSize = strlen(p);

    ssize_t writeSize = write(fd, p, pSize);

    if (writeSize == -1) {
        perror(tmpFilePath);
        close(fd);
        return PPKG_ERROR;
    }

    close(fd);

    if ((size_t)writeSize != pSize) {
        fprintf(stderr, "not fully written to %s\n", tmpFilePath);
        return PPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    if (chmod(tmpFilePath, S_IRWXU) != 0) {
        perror(tmpFilePath);
        return PPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    size_t   outFilePathLength = ppkgHomeDIRLength + 22U;
    char     outFilePath[outFilePathLength];
    snprintf(outFilePath, outFilePathLength, "%s/url-transform.sample", ppkgHomeDIR);

    if (rename(tmpFilePath, outFilePath) != 0) {
        if (errno == EXDEV) {
            ret = ppkg_copy_file(tmpFilePath, outFilePath);

            if (ret != PPKG_OK) {
                return ret;
            }
        } else {
            perror(tmpFilePath);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    fprintf(stderr, "%surl-transform sample has been written into %s%s\n\n", COLOR_GREEN, outFilePath, COLOR_OFF);

    outFilePath[outFilePathLength - 9] = '\0';

    fprintf(stderr, "%sYou can rename url-transform.sample to url-transform then edit it to meet your needs.\n\nTo apply this, you should run 'export PPKG_URL_TRANSFORM=%s' in your terminal.\n%s", COLOR_GREEN, outFilePath, COLOR_OFF);

    return ppkg_rm_r(sessionDIR, false);
}
