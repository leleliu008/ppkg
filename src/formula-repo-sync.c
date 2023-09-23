#include <time.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

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

    const char * branchName = formulaRepo->branch;
    size_t       branchNameLength = strlen(branchName);

    size_t   remoteRefPathLength = branchNameLength + 12U;
    char     remoteRefPath[remoteRefPathLength];
    snprintf(remoteRefPath, remoteRefPathLength, "refs/heads/%s", branchName);

    size_t   remoteTrackingRefPathLength = branchNameLength + 21U;
    char     remoteTrackingRefPath[remoteTrackingRefPathLength];
    snprintf(remoteTrackingRefPath, remoteTrackingRefPathLength, "refs/remotes/origin/%s", branchName);

    int ret = ppkg_git_sync(formulaRepo->path, formulaRepo->url, remoteRefPath, remoteTrackingRefPath, branchName);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    char ts[11];
    snprintf(ts, 11, "%ld", time(NULL));

    return ppkg_formula_repo_config_write(formulaRepo->path, formulaRepo->url, formulaRepo->branch, formulaRepo->pinned, formulaRepo->enabled, formulaRepo->timestamp_created, ts);
}
