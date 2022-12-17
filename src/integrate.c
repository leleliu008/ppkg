#include <string.h>
#include <sys/stat.h>

#include "core/http.h"
#include "core/fs.h"
#include "ppkg.h"

int ppkg_integrate_zsh_completion(const char * outputDir, bool verbose) {
    const char * url = "https://raw.githubusercontent.com/leleliu008/ppkg/master/zsh-completion/_ppkg";

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL || strcmp(userHomeDir, "") == 0) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    size_t  zshCompletionDirLength = userHomeDirLength + 18;
    char    zshCompletionDir[zshCompletionDirLength];
    memset (zshCompletionDir, 0, zshCompletionDirLength);
    sprintf(zshCompletionDir, "%s/.ppkg/zsh_completion", userHomeDir);

    if (!exists_and_is_a_directory(zshCompletionDir)) {
        if (mkdir(zshCompletionDir, S_IRWXU) != 0) {
            perror(zshCompletionDir);
            return PPKG_ERROR;
        }
    }

    size_t  zshCompletionFilePathLength = zshCompletionDirLength + 2;
    char    zshCompletionFilePath[zshCompletionFilePathLength];
    memset (zshCompletionFilePath, 0, zshCompletionFilePathLength);
    sprintf(zshCompletionFilePath, "%s/_ppkg", zshCompletionDir);

    if (http_fetch_to_file(url, zshCompletionFilePath, verbose, verbose) != 0) {
        return PPKG_NETWORK_ERROR;
    }

    (void)outputDir;
    return PPKG_OK;
}

int ppkg_integrate_bash_completion(const char * outputDir, bool verbose) {
    (void)outputDir;
    (void)verbose;
    return PPKG_OK;
}

int ppkg_integrate_fish_completion(const char * outputDir, bool verbose) {
    (void)outputDir;
    (void)verbose;
    return PPKG_OK;
}
