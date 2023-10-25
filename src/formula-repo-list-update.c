#include <stdio.h>
#include <string.h>

#include "ppkg.h"

int ppkg_formula_repo_list_update() {
    PPKGFormulaRepoList * formulaRepoList = NULL;

    int ret = ppkg_formula_repo_list(&formulaRepoList);

    if (ret == PPKG_OK) {
        bool officalCoreIsThere = false;

        for (size_t i = 0U; i < formulaRepoList->size; i++) {
            PPKGFormulaRepo * formulaRepo = formulaRepoList->repos[i];

            if (strcmp(formulaRepo->name, "offical-core") == 0) {
                officalCoreIsThere = true;
            }

            ret = ppkg_formula_repo_sync(formulaRepo);

            if (ret != PPKG_OK) {
                break;
            }
        }

        ppkg_formula_repo_list_free(formulaRepoList);

        if (!officalCoreIsThere) {
            const char * const formulaRepoUrl = "https://github.com/leleliu008/ppkg-formula-repository-offical-core";
            ret = ppkg_formula_repo_add("offical-core", formulaRepoUrl, "master", false, true);
        }
    }

    return ret;
}
