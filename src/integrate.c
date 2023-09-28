#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "ppkg.h"

int ppkg_integrate_zsh_completion(const char * outputDIR, bool verbose) {
    char   ppkgHomeDIR[PATH_MAX] = {0};
    size_t ppkgHomeDIRLength;

    int ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    ///////////////////////////////////////////////////////////////////////////////////////

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

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t   tmpFilePathLength = sessionDIRLength + 7U;
    char     tmpFilePath[tmpFilePathLength];
    snprintf(tmpFilePath, tmpFilePathLength, "%s/_ppkg", sessionDIR);

    const char * const url = "https://raw.githubusercontent.com/leleliu008/ppkg/master/ppkg-zsh-completion";

    ret = ppkg_http_fetch_to_file(url, tmpFilePath, verbose, verbose);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t   defaultOutputDIRLength = ppkgHomeDIRLength + 26U;
    char     defaultOutputDIR[defaultOutputDIRLength];
    snprintf(defaultOutputDIR, defaultOutputDIRLength, "%s/share/zsh/site-functions", ppkgHomeDIR);

    size_t outputDIRLength;

    if (outputDIR == NULL) {
        outputDIR       = defaultOutputDIR;
        outputDIRLength = defaultOutputDIRLength;
    } else {
        outputDIRLength = strlen(outputDIR);
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    ret = ppkg_mkdir_p(outputDIR, verbose);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t   outputFilePathLength = outputDIRLength + 7U;
    char     outputFilePath[outputFilePathLength];
    snprintf(outputFilePath, outputFilePathLength, "%s/_ppkg", outputDIR);

    if (rename(tmpFilePath, outputFilePath) != 0) {
        if (errno == EXDEV) {
            ret = ppkg_copy_file(tmpFilePath, outputFilePath);

            if (ret != PPKG_OK) {
                return ret;
            }
        } else {
            perror(outputFilePath);
            return PPKG_ERROR;
        }
    }

    printf("zsh completion script for ppkg has been written to %s\n", outputFilePath);
    return PPKG_OK;
}

int ppkg_integrate_bash_completion(const char * outputDIR, bool verbose) {
    (void)outputDIR;
    (void)verbose;
    return PPKG_OK;
}

int ppkg_integrate_fish_completion(const char * outputDIR, bool verbose) {
    (void)outputDIR;
    (void)verbose;
    return PPKG_OK;
}
