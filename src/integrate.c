#include <string.h>
#include <sys/stat.h>

#include "core/http.h"
#include "core/fs.h"
#include "ppkg.h"

int ppkg_integrate_zsh_completion(const char * outputDir, bool verbose) {
    const char * url = "https://raw.githubusercontent.com/leleliu008/ppkg/master/zsh-completion/_ppkg";

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    ////////////////////////////////////////////////////////////////

    size_t  ppkgHomeDirLength = userHomeDirLength + 7;
    char    ppkgHomeDir[ppkgHomeDirLength];
    memset (ppkgHomeDir, 0, ppkgHomeDirLength);
    sprintf(ppkgHomeDir, "%s/.ppkg", userHomeDir);

    if (!exists_and_is_a_directory(ppkgHomeDir)) {
        if (mkdir(ppkgHomeDir, S_IRWXU) != 0) {
            perror(ppkgHomeDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////

    size_t  zshCompletionDirLength = ppkgHomeDirLength + 16;
    char    zshCompletionDir[zshCompletionDirLength];
    memset (zshCompletionDir, 0, zshCompletionDirLength);
    sprintf(zshCompletionDir, "%s/zsh_completion", ppkgHomeDir);

    if (!exists_and_is_a_directory(zshCompletionDir)) {
        if (mkdir(zshCompletionDir, S_IRWXU) != 0) {
            perror(zshCompletionDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////

    size_t  zshCompletionFilePathLength = zshCompletionDirLength + 7;
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
