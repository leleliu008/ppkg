#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <limits.h>
#include <sys/stat.h>

#include "core/http.h"

#include "sha256sum.h"

#include "ppkg.h"

static int package_name_callback(const char * packageName, size_t i, const void * payload) {
    return ppkg_fetch(packageName, *((bool*)payload));
}

static int ppkg_fetch_git(const char * packageName, PPKGFormula * formula, const char * ppkgDownloadsDIR, size_t ppkgDownloadsDIRLength) {
    size_t   gitRepositoryDIRLength = ppkgDownloadsDIRLength + strlen(packageName) + 6U;
    char     gitRepositoryDIR[gitRepositoryDIRLength];
    snprintf(gitRepositoryDIR, gitRepositoryDIRLength, "%s/%s.git", ppkgDownloadsDIR, packageName);

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
    
    return ppkg_git_sync(gitRepositoryDIR, formula->git_url, remoteRef, "refs/remotes/origin/master", "master");
}

static int ppkg_fetch_file(const char * url, const char * uri, const char * expectedSHA256SUM, const char * ppkgDownloadsDIR, size_t ppkgDownloadsDIRLength, bool verbose) {
    char fileNameExtension[21] = {0};

    int ret = ppkg_examine_file_extension_from_url(url, fileNameExtension, 20);

    if (ret != PPKG_OK) {
        return ret;
    }

    printf("==========>> fileNameExtension = %s\n", fileNameExtension);

    size_t   fileNameLength = strlen(expectedSHA256SUM) + strlen(fileNameExtension) + 1U;
    char     fileName[fileNameLength];
    snprintf(fileName, fileNameLength, "%s%s", expectedSHA256SUM, fileNameExtension);

    size_t   filePathLength = ppkgDownloadsDIRLength + fileNameLength + 1U;
    char     filePath[filePathLength];
    snprintf(filePath, filePathLength, "%s/%s", ppkgDownloadsDIR, fileName);

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

    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t   ppkgDownloadsDIRLength = ppkgHomeDIRLength + 11U;
    char     ppkgDownloadsDIR[ppkgDownloadsDIRLength];
    snprintf(ppkgDownloadsDIR, ppkgDownloadsDIRLength, "%s/downloads", ppkgHomeDIR);

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
        ret = ppkg_fetch_git(packageName, formula, ppkgDownloadsDIR, ppkgDownloadsDIRLength);
    } else {
        if (formula->src_is_dir) {
            fprintf(stderr, "src_url is point to local dir, so no need to fetch.\n");
        } else {
            ret = ppkg_fetch_file(formula->src_url, formula->src_uri, formula->src_sha, ppkgDownloadsDIR, ppkgDownloadsDIRLength, verbose);
        }
    }

    if (ret != PPKG_OK) {
        goto finalize;
    }

    if (formula->fix_url != NULL) {
        ret = ppkg_fetch_file(formula->fix_url, formula->fix_uri, formula->fix_sha, ppkgDownloadsDIR, ppkgDownloadsDIRLength, verbose);

        if (ret != PPKG_OK) {
            goto finalize;
        }
    }

    if (formula->res_url != NULL) {
        ret = ppkg_fetch_file(formula->res_url, formula->res_uri, formula->res_sha, ppkgDownloadsDIR, ppkgDownloadsDIRLength, verbose);

        if (ret != PPKG_OK) {
            goto finalize;
        }
    }

finalize:
    ppkg_formula_free(formula);
    return ret;
}
