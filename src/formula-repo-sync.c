#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "core/log.h"
#include "ppkg.h"

int ppkg_formula_repo_sync_(const char * formulaRepoName) {
    PPKGFormulaRepo * formulaRepo = NULL;

    int ret = ppkg_formula_repo_lookup(formulaRepoName, &formulaRepo);

    if (ret == PPKG_OK) {
        ret = ppkg_formula_repo_sync(formulaRepo);
    }

    ppkg_formula_repo_free(formulaRepo);

    return ret;
}

int ppkg_formula_repo_sync(PPKGFormulaRepo * formulaRepo) {
    if (formulaRepo == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (formulaRepo->pinned) {
        fprintf(stderr, "'%s' formula repo was pinned, skipped.\n", formulaRepo->name);
        return PPKG_OK;
    }

    if (isatty(STDOUT_FILENO)) {
        printf("%s%s%s\n", COLOR_PURPLE, "==> Updating formula repo", COLOR_OFF);
    } else {
        printf("=== Updating formula repo\n");
    }

    ppkg_formula_repo_info(formulaRepo);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    size_t refspecLength = (strlen(formulaRepo->branch) << 1) + 33U;
    char   refspec[refspecLength];
    snprintf(refspec, refspecLength, "refs/heads/%s:refs/remotes/origin/%s", formulaRepo->branch, formulaRepo->branch);

    if (ppkg_fetch_via_git(formulaRepo->path, formulaRepo->url, refspec, formulaRepo->branch) != 0) {
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    char ts[11];
    snprintf(ts, 11, "%ld", time(NULL));

    return ppkg_formula_repo_config_write(formulaRepo->path, formulaRepo->url, formulaRepo->branch, formulaRepo->pinned, formulaRepo->enabled, formulaRepo->timestamp_created, ts);
}
