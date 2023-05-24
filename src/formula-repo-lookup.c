#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "ppkg.h"

int ppkg_formula_repo_lookup(const char * formulaRepoName, PPKGFormulaRepo * * formulaRepoPP) {
    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t formulaRepoNameLength = strlen(formulaRepoName);

    size_t formulaRepoDirPathLength = userHomeDirLength + formulaRepoNameLength + 16U;
    char   formulaRepoDirPath[formulaRepoDirPathLength];
    snprintf(formulaRepoDirPath, formulaRepoDirPathLength, "%s/.ppkg/repos.d/%s", userHomeDir, formulaRepoName);

    struct stat st;

    if (stat(formulaRepoDirPath, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", formulaRepoDirPath);
            return PPKG_ERROR;
        }
    } else {
        return PPKG_ERROR_FORMULA_REPO_NOT_FOUND;
    }

    size_t formulaRepoConfigFilePathLength = formulaRepoDirPathLength + 24U;
    char   formulaRepoConfigFilePath[formulaRepoConfigFilePathLength];
    snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathLength, "%s/.ppkg-formula-repo.yml", formulaRepoDirPath);

    if (!((stat(formulaRepoConfigFilePath, &st) == 0) && S_ISREG(st.st_mode))) {
        return PPKG_ERROR_FORMULA_REPO_NOT_FOUND;
    }

    PPKGFormulaRepo * formulaRepo = NULL;

    int ret = ppkg_formula_repo_parse(formulaRepoConfigFilePath, &formulaRepo);

    if (ret != PPKG_OK) {
        return ret;
    }

    formulaRepo->name = strdup(formulaRepoName);

    if (formulaRepo->name == NULL) {
        ppkg_formula_repo_free(formulaRepo);
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    formulaRepo->path = strdup(formulaRepoDirPath);

    if (formulaRepo->path == NULL) {
        ppkg_formula_repo_free(formulaRepo);
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    (*formulaRepoPP) = formulaRepo;
    return PPKG_OK;
}
