#include <stdio.h>
#include <string.h>

#include <sys/stat.h>

#include "ppkg.h"

int ppkg_formula_locate(const char * packageName, char ** out) {
    int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    char osType[31] = {0};

    if (sysinfo_type(osType, 30) != 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    PPKGFormulaRepoList * formulaRepoList = NULL;

    ret = ppkg_formula_repo_list(&formulaRepoList);

    if (ret != PPKG_OK) {
        ppkg_formula_repo_list_free(formulaRepoList);
        return ret;
    }

    struct stat st;

    size_t packageNameLength = strlen(packageName);

    for (size_t i = 0U; i < formulaRepoList->size; i++) {
        char * formulaRepoPath = formulaRepoList->repos[i]->path;

        const char* a[2] = { osType, ""};

        for (int j = 0; j < 2; j++) {
            size_t formulaFilePathLength = strlen(formulaRepoPath) + strlen(a[j]) + packageNameLength + 15U;
            char   formulaFilePath[formulaFilePathLength];

            ret = snprintf(formulaFilePath, formulaFilePathLength, "%s/formula/%s/%s.yml", formulaRepoPath, a[j], packageName);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

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
    }

    ppkg_formula_repo_list_free(formulaRepoList);
    return PPKG_ERROR_PACKAGE_NOT_AVAILABLE;
}