#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <git2.h>

#include "ppkg.h"

int ppkg_formula_repo_create(const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled) {
    if (formulaRepoName == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    size_t formulaRepoNameLength = strlen(formulaRepoName);

    if (formulaRepoNameLength == 0) {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    if (formulaRepoUrl == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    size_t formulaRepoUrlLength = strlen(formulaRepoUrl);

    if (formulaRepoUrlLength == 0) {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    if (branchName == NULL || branchName[0] == '\0') {
        branchName = (char*)"master";
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

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

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t formulaRepoRootDirLength = ppkgHomeDirLength + 9U;
    char   formulaRepoRootDir[formulaRepoRootDirLength];
    snprintf(formulaRepoRootDir, formulaRepoRootDirLength, "%s/repos.d", ppkgHomeDir);

    if (stat(formulaRepoRootDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", formulaRepoRootDir);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(formulaRepoRootDir, S_IRWXU) != 0) {
            perror(formulaRepoRootDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t formulaRepoDirLength = formulaRepoRootDirLength + formulaRepoNameLength + 2U;
    char   formulaRepoDir[formulaRepoDirLength];
    snprintf(formulaRepoDir, formulaRepoDirLength, "%s/%s", formulaRepoRootDir, formulaRepoName);

    if (stat(formulaRepoDir, &st) == 0) {
        fprintf(stderr, "formula repo already exist at: %s\n", formulaRepoDir);
        return PPKG_ERROR_FORMULA_REPO_HAS_EXIST;
    } else {
        if (mkdir(formulaRepoDir, S_IRWXU) != 0) {
            perror(formulaRepoDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t subDirLength = formulaRepoDirLength + 9U;
    char   subDir[subDirLength];
    snprintf(subDir, subDirLength, "%s/formula", formulaRepoDir);

    if (mkdir(subDir, S_IRWXU) != 0) {
        perror(subDir);
        return PPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    git_repository   * gitRepo   = NULL;
    git_remote       * gitRemote = NULL;
    const git_error  * gitError  = NULL;

    git_libgit2_init();

    int ret = git_repository_init(&gitRepo, formulaRepoDir, false);

    if (ret != GIT_OK) {
        gitError = git_error_last();
        fprintf(stderr, "%s\n", gitError->message);
        git_repository_state_cleanup(gitRepo);
        git_repository_free(gitRepo);
        git_libgit2_shutdown();
        return abs(ret) + PPKG_ERROR_LIBGIT2_BASE;
    }

    //https://libgit2.org/libgit2/#HEAD/group/remote/git_remote_create
    ret = git_remote_create(&gitRemote, gitRepo, "origin", formulaRepoUrl);

    if (ret != GIT_OK) {
        gitError = git_error_last();
        fprintf(stderr, "%s\n", gitError->message);
        git_repository_state_cleanup(gitRepo);
        git_repository_free(gitRepo);
        git_libgit2_shutdown();
        return abs(ret) + PPKG_ERROR_LIBGIT2_BASE;
    }

    git_repository_state_cleanup(gitRepo);
    git_repository_free(gitRepo);
    git_remote_free(gitRemote);
    git_libgit2_shutdown();

    ////////////////////////////////////////////////////////////////////////////////////////

    char ts[11];
    snprintf(ts, 11, "%ld", time(NULL));

    return ppkg_formula_repo_config_write(formulaRepoDir, formulaRepoUrl, branchName, pinned, enabled, ts, NULL);
}
