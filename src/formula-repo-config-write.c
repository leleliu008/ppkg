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

    size_t   strLength = strlen(formulaRepoUrl) + strlen(branchName) + strlen(timestamp_created) + strlen(timestamp_updated) + 75U;
    char     str[strLength + 1U];
    snprintf(str, strLength + 1U, "url: %s\nbranch: %s\npinned: %1d\nenabled: %1d\ntimestamp-created: %s\ntimestamp-updated: %s\n", formulaRepoUrl, branchName, pinned, enabled, timestamp_created, timestamp_updated);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    size_t   formulaRepoConfigFilePathLength = strlen(formulaRepoDIRPath) + 24U;
    char     formulaRepoConfigFilePath[formulaRepoConfigFilePathLength];
    snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathLength, "%s/.ppkg-formula-repo.yml", formulaRepoDIRPath);

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

    if ((size_t)writeSize == strLength) {
        return PPKG_OK;
    } else {
        fprintf(stderr, "not fully written to %s\n", formulaRepoConfigFilePath);
        return PPKG_ERROR;
    }
}
