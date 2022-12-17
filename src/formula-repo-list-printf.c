#include <stdio.h>
#include "ppkg.h"

int ppkg_formula_repo_list_printf() {
    PPKGFormulaRepoList * formulaRepoList = NULL;

    int resultCode = ppkg_formula_repo_list_new(&formulaRepoList);

    if (resultCode == 0) {
        for (size_t i = 0; i < formulaRepoList->size; i++) {
            PPKGFormulaRepo * formulaRepo = formulaRepoList->repos[i];
            printf("%s %s %s\n", formulaRepo->name, formulaRepo->url, formulaRepo->branch);
        }
    }

    ppkg_formula_repo_list_free(formulaRepoList);

    return resultCode;
}
