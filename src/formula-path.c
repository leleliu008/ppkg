#include <stdio.h>
#include <string.h>

#include <limits.h>
#include <sys/stat.h>

#include "core/sysinfo.h"

#include "ppkg.h"

int ppkg_formula_path(const char * packageName, const char * targetPlatformName, char out[]) {
    int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    char nativeOSType[31] = {0};

    if (targetPlatformName == NULL || targetPlatformName[0] == '\0') {
        targetPlatformName = nativeOSType;

        if (sysinfo_type(nativeOSType, 30) < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////

    PPKGFormulaRepoList * formulaRepoList = NULL;

    ret = ppkg_formula_repo_list(&formulaRepoList);

    if (ret != PPKG_OK) {
        ppkg_formula_repo_list_free(formulaRepoList);
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    struct stat st;

    size_t packageNameLength = strlen(packageName);

    for (size_t i = 0U; i < formulaRepoList->size; i++) {
        char * formulaRepoPath = formulaRepoList->repos[i]->path;

        size_t formulaFilePath1Capacity = strlen(formulaRepoPath) + strlen(targetPlatformName) + packageNameLength + 15U;
        char   formulaFilePath1[formulaFilePath1Capacity];

        ret = snprintf(formulaFilePath1, formulaFilePath1Capacity, "%s/formula/%s/%s.yml", formulaRepoPath, targetPlatformName, packageName);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (stat(formulaFilePath1, &st) == 0 && S_ISREG(st.st_mode)) {
            ppkg_formula_repo_list_free(formulaRepoList);
            strncpy(out, formulaFilePath1, formulaFilePath1Capacity);
            return PPKG_OK;
        }

        ////////////////////////////////////////////////////////////////

        size_t formulaFilePath2Capacity = strlen(formulaRepoPath) + packageNameLength + 15U;
        char   formulaFilePath2[formulaFilePath2Capacity];

        ret = snprintf(formulaFilePath2, formulaFilePath2Capacity, "%s/formula/%s.yml", formulaRepoPath, packageName);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (stat(formulaFilePath2, &st) == 0 && S_ISREG(st.st_mode)) {
            ppkg_formula_repo_list_free(formulaRepoList);
            strncpy(out, formulaFilePath2, formulaFilePath2Capacity);
            return PPKG_OK;
        }
    }

    ppkg_formula_repo_list_free(formulaRepoList);
    return PPKG_ERROR_PACKAGE_NOT_AVAILABLE;
}
