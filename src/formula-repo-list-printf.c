#include <stdio.h>
#include <unistd.h>
#include "ppkg.h"

int ppkg_formula_repo_list_printf() {
    PPKGFormulaRepoList * formulaRepoList = NULL;

    int ret = ppkg_formula_repo_list(&formulaRepoList);

    if (ret == PPKG_OK) {
        for (size_t i = 0; i < formulaRepoList->size; i++) {
            if (i > 0) {
                if (isatty(STDOUT_FILENO)) {
                    printf("\n");
                } else {
                    printf("---\n");
                }
            }

            ppkg_formula_repo_info(formulaRepoList->repos[i]);
        }

        ppkg_formula_repo_list_free(formulaRepoList);
    }

    return ret;
}
