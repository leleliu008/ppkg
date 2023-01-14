#include <stdio.h>
#include <string.h>

#include "core/fs.h"
#include "ppkg.h"

int ppkg_formula_path(const char * packageName, char ** out) {
    int resultCode = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t  ppkgHomeDirLength = userHomeDirLength + 7; 
    char    ppkgHomeDir[ppkgHomeDirLength];
    memset (ppkgHomeDir, 0, ppkgHomeDirLength);
    snprintf(ppkgHomeDir, ppkgHomeDirLength, "%s/.ppkg", userHomeDir);

    if (!exists_and_is_a_directory(ppkgHomeDir)) {
        return PPKG_FORMULA_REPO_NOT_EXIST;
    }

    PPKGFormulaRepoList * formulaRepoList = NULL;

    resultCode = ppkg_formula_repo_list_new(&formulaRepoList);

    if (resultCode != PPKG_OK) {
        ppkg_formula_repo_list_free(formulaRepoList);
        return resultCode;
    }

    for (size_t i = 0; i < formulaRepoList->size; i++) {
        char *  formulaRepoPath = formulaRepoList->repos[i]->path;

        size_t  formulaFilePathLength =  strlen(formulaRepoPath) + strlen(packageName) + 15;
        char *  formulaFilePath = (char*)calloc(formulaFilePathLength, sizeof(char));

        if (formulaFilePath == NULL) {
            ppkg_formula_repo_list_free(formulaRepoList);
            return PPKG_ERROR_ALLOCATE_MEMORY_FAILED;
        }

        snprintf(formulaFilePath, formulaFilePathLength, "%s/formula/%s.yml", formulaRepoPath, packageName);

        if (exists_and_is_a_regular_file(formulaFilePath)) {
            (*out) = formulaFilePath;
            ppkg_formula_repo_list_free(formulaRepoList);
            return PPKG_OK;
        } else {
            free(formulaFilePath);
            formulaFilePath = NULL;
        }
    }

    ppkg_formula_repo_list_free(formulaRepoList);
    return PPKG_PACKAGE_IS_NOT_AVAILABLE;
}
