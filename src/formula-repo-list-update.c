#include <stdio.h>
#include <string.h>

#include "ppkg.h"

int ppkg_formula_repo_list_update() {
    PPKGFormulaRepoList * formulaRepoList = NULL;

    int resultCode = ppkg_formula_repo_list_new(&formulaRepoList);

    if (resultCode == PPKG_OK) {
        bool officalCoreIsThere = false;

        for (size_t i = 0; i < formulaRepoList->size; i++) {
            PPKGFormulaRepo * formulaRepo = formulaRepoList->repos[i];

            if (strcmp(formulaRepo->name, "offical-core") == 0) {
                officalCoreIsThere = true;
            }

            if (formulaRepo->pinned) {
                fprintf(stderr, "[%s] formula repo was pinned, skipped.\n", formulaRepo->name);
            } else {
                resultCode = ppkg_formula_repo_update(formulaRepo);

                if (resultCode != PPKG_OK) {
                    break;
                }
            }
        }

        if (!officalCoreIsThere) {
            resultCode = ppkg_formula_repo_add("offical-core", "https://github.com/leleliu008/ppkg-formula-repository-offical-core", "master");
        }
    }

    ppkg_formula_repo_list_free(formulaRepoList);

    return resultCode;
}
