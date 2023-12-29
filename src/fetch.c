#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <limits.h>
#include <sys/stat.h>

#include "core/http.h"

#include "sha256sum.h"

#include "ppkg.h"

static int ppkg_fetch_git(const char * packageName, PPKGFormula * formula, const char * ppkgDownloadsDIR, size_t ppkgDownloadsDIRLength) {
    size_t gitRepositoryDIRCapacity = ppkgDownloadsDIRLength + strlen(packageName) + 6U;
    char   gitRepositoryDIR[gitRepositoryDIRCapacity];

    int ret = snprintf(gitRepositoryDIR, gitRepositoryDIRCapacity, "%s/%s.git", ppkgDownloadsDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    struct stat st;

    if (stat(gitRepositoryDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "%s was expected to be a directory, but it was not.\n", gitRepositoryDIR);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(gitRepositoryDIR, S_IRWXU) != 0) {
            perror(gitRepositoryDIR);
            return PPKG_ERROR;
        }
    }

    const char * remoteRef;

    if (formula->git_sha == NULL) {
        remoteRef = (formula->git_ref == NULL) ? "HEAD" : formula->git_ref;
    } else {
        remoteRef = formula->git_sha;
    }
    
    return ppkg_git_sync(gitRepositoryDIR, formula->git_url, remoteRef, "refs/remotes/origin/master", "master", formula->git_nth);
}

static int ppkg_fetch_file(const char * url, const char * uri, const char * expectedSHA256SUM, const char * ppkgDownloadsDIR, const size_t ppkgDownloadsDIRLength, const bool verbose) {
    char fileNameExtension[21] = {0};

    int ret = ppkg_examine_filetype_from_url(url, fileNameExtension, 20);

    if (ret != PPKG_OK) {
        return ret;
    }

    printf("==========>> fileNameExtension = %s\n", fileNameExtension);

    size_t fileNameCapacity = strlen(expectedSHA256SUM) + strlen(fileNameExtension) + 1U;
    char   fileName[fileNameCapacity];

    ret = snprintf(fileName, fileNameCapacity, "%s%s", expectedSHA256SUM, fileNameExtension);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    size_t filePathCapacity = ppkgDownloadsDIRLength + fileNameCapacity + 1U;
    char   filePath[filePathCapacity];

    ret = snprintf(filePath, filePathCapacity, "%s/%s", ppkgDownloadsDIR, fileName);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

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

    ret = ppkg_http_fetch_to_file(url, filePath, verbose, verbose);

    if (ret != PPKG_OK) {
        if (uri != NULL && uri[0] != '\0') {
            ret = ppkg_http_fetch_to_file(uri, filePath, verbose, verbose);
        }
    }

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

int ppkg_fetch(const char * packageName, const char * targetPlatformName, const bool verbose) {
    PPKGFormula * formula = NULL;

    int ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

    if (ret != PPKG_OK) {
        return ret;
    }

    ///////////////////////////////////////////////////////////////

    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t ppkgDownloadsDIRCapacity = ppkgHomeDIRLength + 11U;
    char   ppkgDownloadsDIR[ppkgDownloadsDIRCapacity];

    ret = snprintf(ppkgDownloadsDIR, ppkgDownloadsDIRCapacity, "%s/downloads", ppkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    struct stat st;

    if (stat(ppkgDownloadsDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(ppkgDownloadsDIR) != 0) {
                perror(ppkgDownloadsDIR);
                ppkg_formula_free(formula);
                return PPKG_ERROR;
            }

            if (mkdir(ppkgDownloadsDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(ppkgDownloadsDIR);
                    ppkg_formula_free(formula);
                    return PPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(ppkgDownloadsDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(ppkgDownloadsDIR);
                ppkg_formula_free(formula);
                return PPKG_ERROR;
            }
        }
    }

    ///////////////////////////////////////////////////////////////

    if (formula->src_url == NULL) {
        ret = ppkg_fetch_git(packageName, formula, ppkgDownloadsDIR, ppkgDownloadsDIRCapacity);
    } else {
        if (formula->src_is_dir) {
            fprintf(stderr, "src_url is point to local dir, so no need to fetch.\n");
        } else {
            ret = ppkg_fetch_file(formula->src_url, formula->src_uri, formula->src_sha, ppkgDownloadsDIR, ppkgDownloadsDIRCapacity, verbose);
        }
    }

    if (ret != PPKG_OK) {
        goto finalize;
    }

    if (formula->fix_url != NULL) {
        ret = ppkg_fetch_file(formula->fix_url, formula->fix_uri, formula->fix_sha, ppkgDownloadsDIR, ppkgDownloadsDIRCapacity, verbose);

        if (ret != PPKG_OK) {
            goto finalize;
        }
    }

    if (formula->res_url != NULL) {
        ret = ppkg_fetch_file(formula->res_url, formula->res_uri, formula->res_sha, ppkgDownloadsDIR, ppkgDownloadsDIRCapacity, verbose);

        if (ret != PPKG_OK) {
            goto finalize;
        }
    }

finalize:
    ppkg_formula_free(formula);
    return ret;
}
