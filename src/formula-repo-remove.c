#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "core/rm-r.h"
#include "ppkg.h"

int ppkg_formula_repo_remove(const char * formulaRepoName) {
    if (formulaRepoName == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    size_t formulaRepoNameLength = strlen(formulaRepoName);

    if (formulaRepoNameLength == 0) {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    if (strcmp(formulaRepoName, "offical-core") == 0) {
        fprintf(stderr, "offical-core formula repo is not allowed to delete.\n");
        return PPKG_ERROR;
    }

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t formulaRepoPathLength = userHomeDirLength + formulaRepoNameLength + 16U;
    char   formulaRepoPath[formulaRepoPathLength];
    snprintf(formulaRepoPath, formulaRepoPathLength, "%s/.ppkg/repos.d/%s", userHomeDir, formulaRepoName);

    struct stat st;

    if (stat(formulaRepoPath, &st) != 0) {
        fprintf(stderr, "formula repo not found: %s\n", formulaRepoName);
        return PPKG_ERROR;
    }

    size_t formulaRepoConfigFilePathLength = formulaRepoPathLength + 24U;
    char   formulaRepoConfigFilePath[formulaRepoConfigFilePathLength];
    snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathLength, "%s/.ppkg-formula-repo.yml", formulaRepoPath);

    if (stat(formulaRepoConfigFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
        return rm_r(formulaRepoPath, false);
    } else {
        fprintf(stderr, "formula repo is broken: %s\n", formulaRepoName);
        return PPKG_ERROR;
    }
}
