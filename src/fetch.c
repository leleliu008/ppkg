#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "core/http.h"
#include "core/sha256sum.h"

#include "ppkg.h"


static int package_name_callback(const char * packageName, size_t i, const void * payload) {
    return ppkg_fetch(packageName, *((bool*)payload));
}

static int ppkg_fetch_git(const char * packageName, PPKGFormula * formula, const char * ppkgDownloadsDir, size_t ppkgDownloadsDirLength) {
    size_t   gitRepositoryDirLength = ppkgDownloadsDirLength + strlen(packageName) + 6U;
    char     gitRepositoryDir[gitRepositoryDirLength];
    snprintf(gitRepositoryDir, gitRepositoryDirLength, "%s/%s.git", ppkgDownloadsDir, packageName);

    struct stat st;

    if (stat(gitRepositoryDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", gitRepositoryDir);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(gitRepositoryDir, S_IRWXU) != 0) {
            perror(gitRepositoryDir);
            return PPKG_ERROR;
        }
    }

    if (formula->git_sha == NULL) {
        if (formula->git_ref == NULL) {
            return ppkg_fetch_via_git(gitRepositoryDir, formula->git_url, "refs/heads/master:refs/remotes/origin/master", "master");
        } else {
            size_t  refspecLength = strlen(formula->git_ref) + 28U;
            char    refspec[refspecLength];
            snprintf(refspec, refspecLength, "%s:refs/remotes/origin/master", formula->git_ref);

            return ppkg_fetch_via_git(gitRepositoryDir, formula->git_url, refspec, "master");
        }
    } else {
        size_t  refspecLength = strlen(formula->git_sha) + 28U;
        char    refspec[refspecLength];
        snprintf(refspec, refspecLength, "%s:refs/remotes/origin/master", formula->git_sha);

        return ppkg_fetch_via_git(gitRepositoryDir, formula->git_url, refspec, formula->git_sha);
    }
}

static int ppkg_fetch_file(const char * url, const char * expectedSHA256SUM, const char * ppkgDownloadsDir, size_t ppkgDownloadsDirLength, bool verbose) {
    char fileNameExtension[21] = {0};

    int ret = ppkg_examine_file_extension_from_url(fileNameExtension, 20, url);

    if (ret != PPKG_OK) {
        return ret;
    }

    printf("==========>> fileNameExtension = %s\n", fileNameExtension);

    size_t  fileNameLength = strlen(expectedSHA256SUM) + strlen(fileNameExtension) + 1U;
    char    fileName[fileNameLength];
    snprintf(fileName, fileNameLength, "%s%s", expectedSHA256SUM, fileNameExtension);

    size_t  filePathLength = ppkgDownloadsDirLength + fileNameLength + 1U;
    char    filePath[filePathLength];
    snprintf(filePath, filePathLength, "%s/%s", ppkgDownloadsDir, fileName);

    struct stat st;

    if (stat(filePath, &st) == 0 && S_ISREG(st.st_mode)) {
        char actualSHA256SUM[65] = {0};

        int ret = sha256sum_of_file(actualSHA256SUM, filePath);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (strcmp(actualSHA256SUM, expectedSHA256SUM) == 0) {
            fprintf(stderr, "%s already have been fetched.\n", filePath);
            return PPKG_OK;
        }
    }

    ret = http_fetch_to_file(url, filePath, verbose, verbose);

    if (ret != PPKG_OK) {
        return ret;
    }

    char actualSHA256SUM[65] = {0};

    ret = sha256sum_of_file(actualSHA256SUM, filePath);

    if (ret != PPKG_OK) {
        return ret;
    }

    if (strcmp(actualSHA256SUM, expectedSHA256SUM) == 0) {
        return PPKG_OK;
    } else {
        fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", expectedSHA256SUM, actualSHA256SUM);
        return PPKG_ERROR_SHA256_MISMATCH;
    }
}

int ppkg_fetch(const char * packageName, bool verbose) {
    if (packageName == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (packageName[0] == '\0') {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    if (strcmp(packageName, "@all") == 0) {
        return ppkg_list_the_available_packages(package_name_callback, &verbose);
    }

    ///////////////////////////////////////////////////////////////

    PPKGFormula * formula = NULL;

    int ret = ppkg_formula_lookup(packageName, &formula);

    if (ret != PPKG_OK) {
        return ret;
    }

    ///////////////////////////////////////////////////////////////

    const char * const userHomeDir = getenv("HOME");

    if (userHomeDir == NULL || userHomeDir[0] == '\0') {
        ppkg_formula_free(formula);
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    size_t  ppkgDownloadsDirLength = userHomeDirLength + 18U;
    char    ppkgDownloadsDir[ppkgDownloadsDirLength];
    snprintf(ppkgDownloadsDir, ppkgDownloadsDirLength, "%s/.ppkg/downloads", userHomeDir);

    struct stat st;

    if (stat(ppkgDownloadsDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", ppkgDownloadsDir);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(ppkgDownloadsDir, S_IRWXU) != 0) {
            perror(ppkgDownloadsDir);
            ppkg_formula_free(formula);
            return PPKG_ERROR;
        }
    }

    ///////////////////////////////////////////////////////////////

    if (formula->src_url == NULL) {
        ret = ppkg_fetch_git(packageName, formula, ppkgDownloadsDir, ppkgDownloadsDirLength);
    } else {
        if (formula->src_is_dir) {
            fprintf(stderr, "src_url is point to local dir, so no need to fetch.\n");
        } else {
            ret = ppkg_fetch_file(formula->src_url, formula->src_sha, ppkgDownloadsDir, ppkgDownloadsDirLength, verbose);
        }
    }

    if (ret != PPKG_OK) {
        goto finalize;
    }

    if (formula->fix_url != NULL) {
        ret = ppkg_fetch_file(formula->fix_url, formula->fix_sha, ppkgDownloadsDir, ppkgDownloadsDirLength, verbose);

        if (ret != PPKG_OK) {
            goto finalize;
        }
    }

    if (formula->res_url != NULL) {
        ret = ppkg_fetch_file(formula->res_url, formula->res_sha, ppkgDownloadsDir, ppkgDownloadsDirLength, verbose);

        if (ret != PPKG_OK) {
            goto finalize;
        }
    }

finalize:
    ppkg_formula_free(formula);
    return ret;
}
