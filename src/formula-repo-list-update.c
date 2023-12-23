#include <stdio.h>
#include <string.h>

#include "ppkg.h"

int ppkg_formula_repo_list_update() {
    PPKGFormulaRepoList * formulaRepoList = NULL;

    int ret = ppkg_formula_repo_list(&formulaRepoList);

    if (ret == PPKG_OK) {
        bool officialCoreIsThere = false;

        for (size_t i = 0U; i < formulaRepoList->size; i++) {
            PPKGFormulaRepo * formulaRepo = formulaRepoList->repos[i];

            if (strcmp(formulaRepo->name, "official-core") == 0) {
                officialCoreIsThere = true;
            }

            ret = ppkg_formula_repo_sync(formulaRepo);

            if (ret != PPKG_OK) {
                break;
            }
        }

        ppkg_formula_repo_list_free(formulaRepoList);

        if (!officialCoreIsThere) {
            const char * const formulaRepoUrl = "https://github.com/leleliu008/ppkg-formula-repository-official-core";
            ret = ppkg_formula_repo_add("official-core", formulaRepoUrl, "master", false, true);
        }
    }

    return ret;
}
