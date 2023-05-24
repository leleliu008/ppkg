#include <stdio.h>
#include <string.h>

#include "ppkg.h"

int ppkg_formula_repo_config_write(const char * formulaRepoDirPath, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled, const char * timestamp_created, const char * timestamp_updated) {
    if (formulaRepoDirPath == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (branchName == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (timestamp_created == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (timestamp_updated == NULL) {
        timestamp_updated = "";
    }

    size_t strLength = strlen(formulaRepoUrl) + strlen(branchName) + strlen(timestamp_created) + strlen(timestamp_updated) + 75U;
    char   str[strLength + 1];
    snprintf(str, strLength + 1, "url: %s\nbranch: %s\npinned: %1d\nenabled: %1d\ntimestamp-created: %s\ntimestamp-updated: %s\n", formulaRepoUrl, branchName, pinned, enabled, timestamp_created, timestamp_updated);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    size_t formulaRepoConfigFilePathLength = strlen(formulaRepoDirPath) + 24U;
    char   formulaRepoConfigFilePath[formulaRepoConfigFilePathLength];
    snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathLength, "%s/.ppkg-formula-repo.yml", formulaRepoDirPath);

    FILE * file = fopen(formulaRepoConfigFilePath, "w");

    if (file == NULL) {
        perror(formulaRepoConfigFilePath);
        return PPKG_ERROR;
    }

    if (fwrite(str, 1, strLength, file) != strLength || ferror(file)) {
        perror(formulaRepoConfigFilePath);
        fclose(file);
        return PPKG_ERROR;
    }

    fclose(file);
    return PPKG_OK;
}
