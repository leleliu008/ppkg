#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "ppkg.h"

int ppkg_formula_locate(const char * packageName, char ** out) {
    int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    PPKGFormulaRepoList * formulaRepoList = NULL;

    ret = ppkg_formula_repo_list(&formulaRepoList);

    if (ret != PPKG_OK) {
        ppkg_formula_repo_list_free(formulaRepoList);
        return ret;
    }

    struct stat st;

    size_t packageNameLength = strlen(packageName);

    for (size_t i = 0; i < formulaRepoList->size; i++) {
        char * formulaRepoPath = formulaRepoList->repos[i]->path;

        size_t formulaFilePathLength =  strlen(formulaRepoPath) + packageNameLength + 15U;
        char   formulaFilePath[formulaFilePathLength];
        snprintf(formulaFilePath, formulaFilePathLength, "%s/formula/%s.yml", formulaRepoPath, packageName);

        if (stat(formulaFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            ppkg_formula_repo_list_free(formulaRepoList);

            (*out) = strdup(formulaFilePath);

            if (*out == NULL) {
                return PPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return PPKG_OK;
            }
        }
    }

    ppkg_formula_repo_list_free(formulaRepoList);
    return PPKG_ERROR_PACKAGE_NOT_AVAILABLE;
}
