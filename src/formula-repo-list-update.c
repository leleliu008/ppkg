#include <stdio.h>
#include <string.h>

#include "core/sysinfo.h"

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
            char osType[31] = {0};

            if (sysinfo_type(osType, 30) != 0) {
                return PPKG_ERROR;
            }

            char osArch[31] = {0};

            if (sysinfo_arch(osArch, 30) != 0) {
                return PPKG_ERROR;
            }

            size_t   formulaRepoUrlLength = strlen(osType) + strlen(osArch) + 56U;
            char     formulaRepoUrl[formulaRepoUrlLength];
            snprintf(formulaRepoUrl, formulaRepoUrlLength, "https://github.com/leleliu008/ppkg-formula-repository-%s-%s", osType, osArch);

            ret = ppkg_formula_repo_add("offical-core", formulaRepoUrl, "master", false, true);
        }
    }

    return ret;
}
