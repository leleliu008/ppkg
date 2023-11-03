#include <stdio.h>
#include <string.h>

#include <limits.h>
#include <sys/stat.h>

#include "ppkg.h"

int ppkg_formula_repo_lookup(const char * formulaRepoName, PPKGFormulaRepo * * formulaRepoPP) {
    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    int ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t formulaRepoNameLength = strlen(formulaRepoName);

    size_t formulaRepoDIRPathCapacity = ppkgHomeDIRLength + formulaRepoNameLength + 10U;
    char   formulaRepoDIRPath[formulaRepoDIRPathCapacity];

    ret = snprintf(formulaRepoDIRPath, formulaRepoDIRPathCapacity, "%s/repos.d/%s", ppkgHomeDIR, formulaRepoName);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    struct stat st;

    if (stat(formulaRepoDIRPath, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "%s was expected to be a directory, but it was not.\n", formulaRepoDIRPath);
            return PPKG_ERROR;
        }
    } else {
        return PPKG_ERROR_FORMULA_REPO_NOT_FOUND;
    }

    size_t formulaRepoConfigFilePathCapacity = formulaRepoDIRPathCapacity + 24U;
    char   formulaRepoConfigFilePath[formulaRepoConfigFilePathCapacity];

    ret = snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathCapacity, "%s/.ppkg-formula-repo.yml", formulaRepoDIRPath);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (!((stat(formulaRepoConfigFilePath, &st) == 0) && S_ISREG(st.st_mode))) {
        return PPKG_ERROR_FORMULA_REPO_NOT_FOUND;
    }

    PPKGFormulaRepo * formulaRepo = NULL;

    ret = ppkg_formula_repo_parse(formulaRepoConfigFilePath, &formulaRepo);

    if (ret != PPKG_OK) {
        return ret;
    }

    formulaRepo->name = strdup(formulaRepoName);

    if (formulaRepo->name == NULL) {
        ppkg_formula_repo_free(formulaRepo);
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    formulaRepo->path = strdup(formulaRepoDIRPath);

    if (formulaRepo->path == NULL) {
        ppkg_formula_repo_free(formulaRepo);
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    (*formulaRepoPP) = formulaRepo;
    return PPKG_OK;
}
