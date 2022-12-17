#include <stdio.h>

#include "core/fs.h"
#include "core/git.h"
#include "ppkg.h"

int ppkg_formula_repo_list_update() {
    PPKGFormulaRepoList * formulaRepoList = NULL;

    int resultCode = ppkg_formula_repo_list_new(&formulaRepoList);

    if (resultCode == PPKG_OK) {
        for (size_t i = 0; i < formulaRepoList->size; i++) {
            PPKGFormulaRepo * formulaRepo = formulaRepoList->repos[i];
            printf("updating formula repo : %s => %s\n", formulaRepo->name, formulaRepo->url);

            if (exists_and_is_a_directory(formulaRepo->path)) {
                resultCode = do_git_pull(formulaRepo->path, NULL, NULL);
            } else {
                resultCode = do_git_clone(formulaRepo->url, formulaRepo->path);
            }
        }
    }

    ppkg_formula_repo_list_free(formulaRepoList);

    return resultCode;
}
