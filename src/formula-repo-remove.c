#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>
#include <sys/stat.h>

#include "ppkg.h"

int ppkg_formula_repo_remove(const char * formulaRepoName) {
    if (formulaRepoName == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (formulaRepoName[0] == '\0') {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    if (strcmp(formulaRepoName, "offical-core") == 0) {
        fprintf(stderr, "offical-core formula repo is not allowed to delete.\n");
        return PPKG_ERROR;
    }

    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    int ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t   formulaRepoPathLength = ppkgHomeDIRLength + strlen(formulaRepoName) + 10U;
    char     formulaRepoPath[formulaRepoPathLength];
    snprintf(formulaRepoPath, formulaRepoPathLength, "%s/repos.d/%s", ppkgHomeDIR, formulaRepoName);

    struct stat st;

    if (stat(formulaRepoPath, &st) != 0) {
        fprintf(stderr, "formula repo not found: %s\n", formulaRepoName);
        return PPKG_ERROR;
    }

    size_t   formulaRepoConfigFilePathLength = formulaRepoPathLength + 24U;
    char     formulaRepoConfigFilePath[formulaRepoConfigFilePathLength];
    snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathLength, "%s/.ppkg-formula-repo.yml", formulaRepoPath);

    if (stat(formulaRepoConfigFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
        return ppkg_rm_r(formulaRepoPath, false);
    } else {
        fprintf(stderr, "formula repo is broken: %s\n", formulaRepoName);
        return PPKG_ERROR;
    }
}
