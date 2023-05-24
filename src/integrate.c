#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "core/http.h"
#include "ppkg.h"

int ppkg_integrate_zsh_completion(const char * outputDir, bool verbose) {
    const char * url = "https://raw.githubusercontent.com/leleliu008/ppkg/master/ppkg-zsh-completion";

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    ////////////////////////////////////////////////////////////////

    struct stat st;

    size_t ppkgHomeDirLength = userHomeDirLength + 7U;
    char   ppkgHomeDir[ppkgHomeDirLength];
    snprintf(ppkgHomeDir, ppkgHomeDirLength, "%s/.ppkg", userHomeDir);

    if (stat(ppkgHomeDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", ppkgHomeDir);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(ppkgHomeDir, S_IRWXU) != 0) {
            perror(ppkgHomeDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////

    size_t zshCompletionDirLength = ppkgHomeDirLength + 16U;
    char   zshCompletionDir[zshCompletionDirLength];
    snprintf(zshCompletionDir, zshCompletionDirLength, "%s/zsh_completion", ppkgHomeDir);

    if (stat(zshCompletionDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", zshCompletionDir);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(zshCompletionDir, S_IRWXU) != 0) {
            perror(zshCompletionDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////

    size_t zshCompletionFilePathLength = zshCompletionDirLength + 7U;
    char   zshCompletionFilePath[zshCompletionFilePathLength];
    snprintf(zshCompletionFilePath, zshCompletionFilePathLength, "%s/_ppkg", zshCompletionDir);

    int ret = http_fetch_to_file(url, zshCompletionFilePath, verbose, verbose);

    if (ret != PPKG_OK) {
        return ret;
    }

    if (outputDir == NULL) {
        return PPKG_OK;
    }

    if (stat(outputDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", outputDir);
            return PPKG_ERROR;
        }
    } else {
        fprintf(stderr, "'%s\n' directory was expected to be exist, but it was not.\n", outputDir);
        return PPKG_ERROR;
    }

    size_t destFilePathLength = strlen(outputDir) + 7U;
    char   destFilePath[destFilePathLength];
    snprintf(destFilePath, destFilePathLength, "%s/_ppkg", outputDir);

    if (symlink(zshCompletionFilePath, destFilePath) != 0) {
        perror(destFilePath);
        return PPKG_ERROR;
    } else {
        return PPKG_OK;
    }
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
