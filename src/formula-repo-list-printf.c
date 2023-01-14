#include <stdio.h>
#include "ppkg.h"

int ppkg_formula_repo_list_printf() {
    PPKGFormulaRepoList * formulaRepoList = NULL;

    int resultCode = ppkg_formula_repo_list_new(&formulaRepoList);

    if (resultCode == PPKG_OK) {
        for (size_t i = 0; i < formulaRepoList->size; i++) {
            if (i > 0) {
                printf("---\n");
            }

            ppkg_formula_repo_dump(formulaRepoList->repos[i]);
        }
    }

    ppkg_formula_repo_list_free(formulaRepoList);

    return resultCode;
}
