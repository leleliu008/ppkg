#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "core/zlib-flate.h"
#include "core/log.h"
#include "core/fs.h"
#include "ppkg.h"

int ppkg_formula_repo_update(PPKGFormulaRepo * formulaRepo) {
    if (isatty(STDOUT_FILENO)) {
        printf("%s%s%s\n", COLOR_PURPLE, "=== Updating formula repo", COLOR_OFF);
    } else {
        printf("=== Updating formula repo\n");
    }

    ppkg_formula_repo_dump(formulaRepo);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    size_t  refspecLength = (strlen(formulaRepo->branch) << 1) + 33;
    char    refspec[refspecLength];
    memset (refspec, 0, refspecLength);
    snprintf(refspec, refspecLength, "refs/heads/%s:refs/remotes/origin/%s", formulaRepo->branch, formulaRepo->branch);

    if (ppkg_fetch_via_git(formulaRepo->path, formulaRepo->url, refspec, formulaRepo->branch) != 0) {
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    char ts[11];
    memset(ts, 0, 11);
    snprintf(ts, 11, "%ld", time(NULL));

    const char * pinnedValue;
    size_t       pinnedValueLength;

    if (formulaRepo->pinned) {
        pinnedValue = "yes";
        pinnedValueLength = 3;
    } else {
        pinnedValue = "no";
        pinnedValueLength = 2;
    }

    size_t  strLength = strlen(formulaRepo->url) + strlen(formulaRepo->branch) + strlen(formulaRepo->timestamp_added) + strlen(ts) + pinnedValueLength + 68;
    char    str[strLength];
    memset (str, 0, strLength);
    snprintf(str, strLength, "url: %s\nbranch: %s\npinned: %s\ntimestamp-added: %s\ntimestamp-last-updated: %s\n", formulaRepo->url, formulaRepo->branch, pinnedValue, formulaRepo->timestamp_added, ts);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    size_t formulaRepoConfigFilePathLength = strlen(formulaRepo->path) + 24;
    char   formulaRepoConfigFilePath[formulaRepoConfigFilePathLength];
    memset(formulaRepoConfigFilePath, 0, formulaRepoConfigFilePathLength);
    snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathLength, "%s/.ppkg-formula-repo.dat", formulaRepo->path);

    FILE * file = fopen(formulaRepoConfigFilePath, "wb");

    if (file == NULL) {
        perror(formulaRepoConfigFilePath);
        return PPKG_ERROR;
    }
    if (zlib_deflate_string_to_file(str, strLength - 1, file) == 0) {
        fclose(file);
        return PPKG_OK;
    } else {
        fclose(file);
        return PPKG_ERROR;
    }
}
