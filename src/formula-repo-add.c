#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>

#include "core/log.h"

#include "ppkg.h"

int ppkg_formula_repo_add(const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled) {
    if (formulaRepoName == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (formulaRepoName[0] == '\0') {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    if (formulaRepoUrl == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (formulaRepoUrl[0] == '\0') {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    if (branchName == NULL || branchName[0] == '\0') {
        branchName = (char*)"master";
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    int ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    struct stat st;

    size_t formulaRepoDIRCapacity = ppkgHomeDIRLength + strlen(formulaRepoName) + 10U;
    char   formulaRepoDIR[formulaRepoDIRCapacity];

    ret = snprintf(formulaRepoDIR, formulaRepoDIRCapacity, "%s/repos.d/%s", ppkgHomeDIR, formulaRepoName);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (stat(formulaRepoDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            fprintf(stderr, "formula repo '%s' already exist.", formulaRepoName);
            return PPKG_ERROR_FORMULA_REPO_HAS_EXIST;
        } else {
            fprintf(stderr, "%s was expected to be a directory, but it was not.\n", formulaRepoDIR);
            return PPKG_ERROR;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    size_t ppkgRunDIRCapacity = ppkgHomeDIRLength + 5U;
    char   ppkgRunDIR[ppkgRunDIRCapacity];

    ret = snprintf(ppkgRunDIR, ppkgRunDIRCapacity, "%s/run", ppkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

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

    size_t sessionDIRCapacity = ppkgRunDIRCapacity + 20U;
    char   sessionDIR[sessionDIRCapacity];

    ret = snprintf(sessionDIR, sessionDIRCapacity, "%s/%d", ppkgRunDIR, getpid());

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

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

    printf("Adding formula repo : %s => %s\n", formulaRepoName, formulaRepoUrl);

    size_t branchNameLength = strlen(branchName);

    size_t remoteRefPathLength = branchNameLength + 12U;
    char   remoteRefPath[remoteRefPathLength];

    ret = snprintf(remoteRefPath, remoteRefPathLength, "refs/heads/%s", branchName);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    size_t remoteTrackingRefPathLength = branchNameLength + 21U;
    char   remoteTrackingRefPath[remoteTrackingRefPathLength];

    ret = snprintf(remoteTrackingRefPath, remoteTrackingRefPathLength, "refs/remotes/origin/%s", branchName);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    ret = ppkg_git_sync(sessionDIR, formulaRepoUrl, remoteRefPath, remoteTrackingRefPath, branchName);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    char ts[11];

    ret = snprintf(ts, 11, "%ld", time(NULL));

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    ret = ppkg_formula_repo_config_write(sessionDIR, formulaRepoUrl, branchName, pinned, enabled, ts, NULL);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t formulaRepoRootDIRLength = ppkgHomeDIRLength + 9U;
    char   formulaRepoRootDIR[formulaRepoRootDIRLength];

    ret = snprintf(formulaRepoRootDIR, formulaRepoRootDIRLength, "%s/repos.d", ppkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (lstat(formulaRepoRootDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(formulaRepoRootDIR) != 0) {
                perror(formulaRepoRootDIR);
                return PPKG_ERROR;
            }

            if (mkdir(formulaRepoRootDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(formulaRepoRootDIR);
                    return PPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(formulaRepoRootDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(formulaRepoRootDIR);
                return PPKG_ERROR;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    if (rename(sessionDIR, formulaRepoDIR) == 0) {
        return PPKG_OK;
    } else {
        perror(sessionDIR);
        return PPKG_ERROR;
    }
}
