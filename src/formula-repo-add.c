#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "ppkg.h"

int ppkg_formula_repo_add(const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled) {
    if (formulaRepoName == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    size_t formulaRepoNameLength = strlen(formulaRepoName);

    if (formulaRepoNameLength == 0) {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    if (formulaRepoUrl == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    size_t formulaRepoUrlLength = strlen(formulaRepoUrl);

    if (formulaRepoUrlLength == 0) {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    if (branchName == NULL || branchName[0] == '\0') {
        branchName = (char*)"master";
    }

    size_t branchNameLength = strlen(branchName);

    ///////////////////////////////////////////////////////////////////////////////////////

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    struct stat st;

    size_t ppkgHomeDirLength = userHomeDirLength + 7U;
    char   ppkgHomeDir[ppkgHomeDirLength];
    snprintf(ppkgHomeDir, ppkgHomeDirLength, "%s/.ppkg", userHomeDir);

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

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t formulaRepoRootDirLength = ppkgHomeDirLength + 9U;
    char   formulaRepoRootDir[formulaRepoRootDirLength];
    snprintf(formulaRepoRootDir, formulaRepoRootDirLength, "%s/repos.d", ppkgHomeDir);

    if (stat(formulaRepoRootDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", formulaRepoRootDir);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(formulaRepoRootDir, S_IRWXU) != 0) {
            perror(formulaRepoRootDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t formulaRepoDirLength = formulaRepoRootDirLength + formulaRepoNameLength + 2U;
    char   formulaRepoDir[formulaRepoDirLength];
    snprintf(formulaRepoDir, formulaRepoDirLength, "%s/%s", formulaRepoRootDir, formulaRepoName);

    if (stat(formulaRepoDir, &st) == 0) {
        fprintf(stderr, "formula repo already exist at: %s\n", formulaRepoDir);
        return PPKG_ERROR_FORMULA_REPO_HAS_EXIST;
    } else {
        if (mkdir(formulaRepoDir, S_IRWXU) != 0) {
            perror(formulaRepoDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    printf("Adding formula repo : %s => %s\n", formulaRepoName, formulaRepoUrl);

    size_t refspecLength = (branchNameLength << 1) + 33U;
    char   refspec[refspecLength];
    snprintf(refspec, refspecLength, "refs/heads/%s:refs/remotes/origin/%s", branchName, branchName);

    int ret = ppkg_fetch_via_git(formulaRepoDir, formulaRepoUrl, refspec, branchName);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    char ts[11];
    snprintf(ts, 11, "%ld", time(NULL));

    return ppkg_formula_repo_config_write(formulaRepoDir, formulaRepoUrl, branchName, pinned, enabled, ts, NULL);
}
