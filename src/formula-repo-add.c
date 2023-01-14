#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ppkg.h"
#include "core/fs.h"
#include "core/zlib-flate.h"

int ppkg_formula_repo_add(const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName) {
    if (formulaRepoName == NULL) {
        return PPKG_ARG_IS_NULL;
    }

    size_t formulaRepoNameLength = strlen(formulaRepoName);

    if (formulaRepoNameLength == 0) {
        return PPKG_ARG_IS_EMPTY;
    }

    if (formulaRepoUrl == NULL) {
        return PPKG_ARG_IS_NULL;
    }

    size_t formulaRepoUrlLength = strlen(formulaRepoUrl);

    if (formulaRepoUrlLength == 0) {
        return PPKG_ARG_IS_EMPTY;
    }

    if (branchName == NULL || strcmp(branchName, "") == 0) {
        branchName = (char*)"master";
    }

    size_t branchNameLength = strlen(branchName);

    ///////////////////////////////////////////////////////////////////////////////////////

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t  ppkgHomeDirLength = userHomeDirLength + 7;
    char    ppkgHomeDir[ppkgHomeDirLength];
    memset (ppkgHomeDir, 0, ppkgHomeDirLength);
    snprintf(ppkgHomeDir, ppkgHomeDirLength, "%s/.ppkg", userHomeDir);

    if (!exists_and_is_a_directory(ppkgHomeDir)) {
        if (mkdir(ppkgHomeDir, S_IRWXU) != 0) {
            perror(ppkgHomeDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t  formulaRepoRootDirLength = ppkgHomeDirLength + 9;
    char    formulaRepoRootDir[formulaRepoRootDirLength];
    memset (formulaRepoRootDir, 0, formulaRepoRootDirLength);
    snprintf(formulaRepoRootDir, formulaRepoRootDirLength, "%s/repos.d", ppkgHomeDir);

    if (!exists_and_is_a_directory(formulaRepoRootDir)) {
        if (mkdir(formulaRepoRootDir, S_IRWXU) != 0) {
            perror(formulaRepoRootDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t  formulaRepoDirLength = formulaRepoRootDirLength + formulaRepoNameLength + 2;
    char    formulaRepoDir[formulaRepoDirLength];
    memset (formulaRepoDir, 0, formulaRepoDirLength);
    snprintf(formulaRepoDir, formulaRepoDirLength, "%s/%s", formulaRepoRootDir, formulaRepoName);

    if (!exists_and_is_a_directory(formulaRepoDir)) {
        if (mkdir(formulaRepoDir, S_IRWXU) != 0) {
            perror(formulaRepoDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    printf("Adding formula repo : %s => %s\n", formulaRepoName, formulaRepoUrl);

    size_t  refspecLength = (branchNameLength << 1) + 33;
    char    refspec[refspecLength];
    memset (refspec, 0, refspecLength);
    snprintf(refspec, refspecLength, "refs/heads/%s:refs/remotes/origin/%s", branchName, branchName);

    if (ppkg_fetch_via_git(formulaRepoDir, formulaRepoUrl, refspec, branchName) != 0) {
        return PPKG_ERROR;
    }

    size_t formulaRepoConfigFilePathLength = formulaRepoDirLength + 24;
    char   formulaRepoConfigFilePath[formulaRepoConfigFilePathLength];
    memset(formulaRepoConfigFilePath, 0, formulaRepoConfigFilePathLength);
    snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathLength, "%s/.ppkg-formula-repo.dat", formulaRepoDir);

    FILE * file = fopen(formulaRepoConfigFilePath, "wb");

    if (file == NULL) {
        perror(formulaRepoConfigFilePath);
        return PPKG_ERROR;
    }

    char ts[11];
    memset(ts, 0, 11);
    snprintf(ts, 11, "%ld", time(NULL));

    size_t  strLength = formulaRepoUrlLength + branchNameLength + strlen(ts) + 45;
    char    str[strLength];
    memset (str, 0, strLength);
    snprintf(str, strLength, "url: %s\nbranch: %s\npinned: no\ntimestamp-added: %s\n", formulaRepoUrl, branchName, ts);

    if (zlib_deflate_string_to_file(str, strLength - 1, file) != 0) {
        fclose(file);

        if (unlink(formulaRepoConfigFilePath) != 0) {
            perror(formulaRepoConfigFilePath);
        }

        return PPKG_ERROR;
    } else {
        fclose(file);
        return PPKG_OK;
    }
}
