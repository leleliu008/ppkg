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

    if (strcmp(formulaRepoName, "official-core") == 0) {
        fprintf(stderr, "official-core formula repo is not allowed to delete.\n");
        return PPKG_ERROR;
    }

    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    int ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t formulaRepoPathCapacity = ppkgHomeDIRLength + strlen(formulaRepoName) + 10U;
    char   formulaRepoPath[formulaRepoPathCapacity];

    ret = snprintf(formulaRepoPath, formulaRepoPathCapacity, "%s/repos.d/%s", ppkgHomeDIR, formulaRepoName);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    struct stat st;

    if (stat(formulaRepoPath, &st) != 0) {
        fprintf(stderr, "formula repo not found: %s\n", formulaRepoName);
        return PPKG_ERROR;
    }

    size_t formulaRepoConfigFilePathCapacity = formulaRepoPathCapacity + 24U;
    char   formulaRepoConfigFilePath[formulaRepoConfigFilePathCapacity];

    ret = snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathCapacity, "%s/.ppkg-formula-repo.yml", formulaRepoPath);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (stat(formulaRepoConfigFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
        return ppkg_rm_rf(formulaRepoPath, false, false);
    } else {
        fprintf(stderr, "formula repo is broken: %s\n", formulaRepoName);
        return PPKG_ERROR;
    }
}
