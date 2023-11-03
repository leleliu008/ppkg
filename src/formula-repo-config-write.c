#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include "ppkg.h"

int ppkg_formula_repo_config_write(const char * formulaRepoDIRPath, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled, const char * timestamp_created, const char * timestamp_updated) {
    if (formulaRepoDIRPath == NULL) {
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

    size_t strCapacity = strlen(formulaRepoUrl) + strlen(branchName) + strlen(timestamp_created) + strlen(timestamp_updated) + 78U;
    char   str[strCapacity];

    int ret = snprintf(str, strCapacity, "url: %s\nbranch: %s\npinned: %1d\nenabled: %1d\ntimestamp-created: %s\ntimestamp-updated: %s\n", formulaRepoUrl, branchName, pinned, enabled, timestamp_created, timestamp_updated);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    int strLength = ret;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    size_t formulaRepoConfigFilePathCapacity = strlen(formulaRepoDIRPath) + 24U;
    char   formulaRepoConfigFilePath[formulaRepoConfigFilePathCapacity];

    ret = snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathCapacity, "%s/.ppkg-formula-repo.yml", formulaRepoDIRPath);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    int fd = open(formulaRepoConfigFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(formulaRepoConfigFilePath);
        return PPKG_ERROR;
    }

    ssize_t writeSize = write(fd, str, strLength);

    if (writeSize == -1) {
        perror(formulaRepoConfigFilePath);
        close(fd);
        return PPKG_ERROR;
    }

    close(fd);

    if (writeSize == strLength) {
        return PPKG_OK;
    } else {
        fprintf(stderr, "not fully written to %s\n", formulaRepoConfigFilePath);
        return PPKG_ERROR;
    }
}
