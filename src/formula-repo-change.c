#include "ppkg.h"

int ppkg_formula_repo_config(const char * formulaRepoName, const char * url, const char * branch, int pinned, int enabled) {
    PPKGFormulaRepo * formulaRepo = NULL;

    int ret = ppkg_formula_repo_lookup(formulaRepoName, &formulaRepo);

    if (ret != PPKG_OK) {
        return ret;
    }

    if (url == NULL) {
        url = formulaRepo->url;
    }

    if (branch == NULL) {
        branch = formulaRepo->branch;
    }

    if (pinned == -1) {
        pinned = formulaRepo->pinned;
    }

    if (enabled == -1) {
        enabled = formulaRepo->enabled;
    }

    ret = ppkg_formula_repo_config_write(formulaRepo->path, url, branch, pinned, enabled, formulaRepo->timestamp_created, formulaRepo->timestamp_updated);

    ppkg_formula_repo_free(formulaRepo);

    return ret;
}
