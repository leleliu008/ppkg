#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/fs.h"
#include "core/rm-r.h"
#include "ppkg.h"

int ppkg_formula_repo_del(const char * formulaRepoName) {
    if (formulaRepoName == NULL) {
        return PPKG_ARG_IS_NULL;
    }

    if (strcmp(formulaRepoName, "") == 0) {
        return PPKG_ARG_IS_EMPTY;
    }

    if (strcmp(formulaRepoName, "offical-core") == 0) {
        fprintf(stderr, "offical-core formula repo is not allowed to delete.\n");
        return PPKG_ERROR;
    }

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (strcmp(userHomeDir, "") == 0) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t  ppkgFormulaRepoDirLength = userHomeDirLength + 15;
    char    ppkgFormulaRepoDir[ppkgFormulaRepoDirLength];
    memset (ppkgFormulaRepoDir, 0, ppkgFormulaRepoDirLength);
    snprintf(ppkgFormulaRepoDir, ppkgFormulaRepoDirLength, "%s/.ppkg/repos.d", userHomeDir);

    size_t  formulaRepoDirLength = ppkgFormulaRepoDirLength + strlen(formulaRepoName) + 2;
    char    formulaRepoDir[formulaRepoDirLength];
    memset (formulaRepoDir, 0, formulaRepoDirLength);
    snprintf(formulaRepoDir, formulaRepoDirLength, "%s/%s", ppkgFormulaRepoDir, formulaRepoName);

    if (exists_and_is_a_directory(formulaRepoDir)) {
        return rm_r(formulaRepoDir, false);
    } else {
        fprintf(stderr, "%s named formula repo is not exist.", formulaRepoName);
        return PPKG_ERROR;
    }
}
