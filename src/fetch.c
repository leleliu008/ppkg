#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>
#include <sys/stat.h>

#include "core/fs.h"
#include "core/http.h"
#include "core/util.h"
#include "core/sha256sum.h"

#include "ppkg.h"

#include <dirent.h>
#include <fnmatch.h>

int ppkg_fetch_all_available_packages(bool verbose) {
    PPKGFormulaRepoList * formulaRepoList = NULL;

    int resultCode = ppkg_formula_repo_list_new(&formulaRepoList);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    for (size_t i = 0; i < formulaRepoList->size; i++) {
        char *  formulaRepoPath  = formulaRepoList->repos[i]->path;

        size_t  formulaDirLength = strlen(formulaRepoPath) + 10;
        char    formulaDir[formulaDirLength];
        memset (formulaDir, 0, formulaDirLength);
        snprintf(formulaDir, formulaDirLength, "%s/formula", formulaRepoPath);

        DIR           * dir;
        struct dirent * dir_entry;

        dir = opendir(formulaDir);

        if (dir == NULL) {
            ppkg_formula_repo_list_free(formulaRepoList);
            perror(formulaDir);
            return PPKG_ERROR;
        }

        while ((dir_entry = readdir(dir))) {
            //puts(dir_entry->d_name);
            if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
                continue;
            }

            int r = fnmatch("*.yml", dir_entry->d_name, 0);

            if (r == 0) {
                size_t  fileNameLength = strlen(dir_entry->d_name);
                char    packageName[fileNameLength];
                memset (packageName, 0, fileNameLength);
                strncpy(packageName, dir_entry->d_name, fileNameLength - 4);

                //printf("%s\n", packageName);
                resultCode = ppkg_fetch(packageName, verbose);

                if (resultCode != PPKG_OK) {
                    ppkg_formula_repo_list_free(formulaRepoList);
                    closedir(dir);
                    return resultCode;
                }
            } else if(r == FNM_NOMATCH) {
                ;
            } else {
                ppkg_formula_repo_list_free(formulaRepoList);
                fprintf(stderr, "fnmatch() error\n");
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        closedir(dir);
    }

    ppkg_formula_repo_list_free(formulaRepoList);

    return PPKG_OK;
}

static int ppkg_fetch_git(const char * packageName, PPKGFormula * formula, const char * ppkgDownloadsDir, size_t ppkgDownloadsDirLength) {
    size_t  gitRepositoryDirLength = ppkgDownloadsDirLength + strlen(packageName) + 6;
    char    gitRepositoryDir[gitRepositoryDirLength];
    memset (gitRepositoryDir, 0, gitRepositoryDirLength);
    snprintf(gitRepositoryDir, gitRepositoryDirLength, "%s/%s.git", ppkgDownloadsDir, packageName);

    if (!exists_and_is_a_directory(gitRepositoryDir)) {
        if (mkdir(gitRepositoryDir, S_IRWXU) != 0) {
            perror(gitRepositoryDir);
            return PPKG_ERROR;
        }
    }

    if (formula->git_sha == NULL) {
        if (formula->git_ref == NULL) {
            return ppkg_fetch_via_git(gitRepositoryDir, formula->git_url, "refs/heads/master:refs/remotes/origin/master", "master");
        } else {
            size_t  refspecLength = strlen(formula->git_ref) + 28;
            char    refspec[refspecLength];
            memset (refspec, 0, refspecLength);
            snprintf(refspec, refspecLength, "%s:refs/remotes/origin/master", formula->git_ref);

            return ppkg_fetch_via_git(gitRepositoryDir, formula->git_url, refspec, "master");
        }
    } else {
        size_t  refspecLength = strlen(formula->git_sha) + 28;
        char    refspec[refspecLength];
        memset (refspec, 0, refspecLength);
        snprintf(refspec, refspecLength, "%s:refs/remotes/origin/master", formula->git_sha);

        return ppkg_fetch_via_git(gitRepositoryDir, formula->git_url, refspec, formula->git_sha);
    }
}

static int ppkg_fetch_file(const char * url, const char * expectedSHA256SUM, const char * ppkgDownloadsDir, size_t ppkgDownloadsDirLength, bool verbose) {
    char fileNameExtension[21] = {0};

    if (get_file_extension_from_url(fileNameExtension, 20, url) < 0) {
        return PPKG_ERROR;
    }

    printf("==========>> fileNameExtension = %s\n", fileNameExtension);

    size_t  fileNameLength = strlen(expectedSHA256SUM) + strlen(fileNameExtension) + 1;
    char    fileName[fileNameLength];
    memset( fileName, 0, fileNameLength);
    snprintf(fileName, fileNameLength, "%s%s", expectedSHA256SUM, fileNameExtension);

    size_t  filePathLength = ppkgDownloadsDirLength + fileNameLength + 1;
    char    filePath[filePathLength];
    memset (filePath, 0, filePathLength);
    snprintf(filePath, filePathLength, "%s/%s", ppkgDownloadsDir, fileName);

    if (exists_and_is_a_regular_file(filePath)) {
        char actualSHA256SUM[65] = {0};

        int resultCode = sha256sum_of_file(actualSHA256SUM, filePath);

        if (resultCode != 0) {
            return PPKG_ERROR;
        }

        if (strcmp(actualSHA256SUM, expectedSHA256SUM) == 0) {
            fprintf(stderr, "%s already have been fetched.\n", filePath);
            return PPKG_OK;
        }
    }

    if (http_fetch_to_file(url, filePath, verbose, verbose) != 0) {
        return PPKG_NETWORK_ERROR;
    }

    char actualSHA256SUM[65] = {0};

    int resultCode = sha256sum_of_file(actualSHA256SUM, filePath);

    if (resultCode != 0) {
        return PPKG_ERROR;
    }

    if (strcmp(actualSHA256SUM, expectedSHA256SUM) == 0) {
        return PPKG_OK;
    } else {
        fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", expectedSHA256SUM, actualSHA256SUM);
        return PPKG_SHA256_MISMATCH;
    }
}

int ppkg_fetch(const char * packageName, bool verbose) {
    if (packageName == NULL) {
        return PPKG_ARG_IS_NULL;
    }

    if (strcmp(packageName, "") == 0) {
        return PPKG_ARG_IS_EMPTY;
    }

    if (strcmp(packageName, "@all") == 0) {
        return ppkg_fetch_all_available_packages(verbose);
    }

    ///////////////////////////////////////////////////////////////

    PPKGFormula * formula = NULL;

    int resultCode = ppkg_formula_parse(packageName, &formula);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    ///////////////////////////////////////////////////////////////

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL || strcmp(userHomeDir, "") == 0) {
        ppkg_formula_free(formula);
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    size_t  ppkgDownloadsDirLength = userHomeDirLength + 18;
    char    ppkgDownloadsDir[ppkgDownloadsDirLength];
    memset (ppkgDownloadsDir, 0, ppkgDownloadsDirLength);
    snprintf(ppkgDownloadsDir, ppkgDownloadsDirLength, "%s/.ppkg/downloads", userHomeDir);

    if (!exists_and_is_a_directory(ppkgDownloadsDir)) {
        if (mkdir(ppkgDownloadsDir, S_IRWXU) != 0) {
            perror(ppkgDownloadsDir);
            ppkg_formula_free(formula);
            return PPKG_ERROR;
        }
    }

    ///////////////////////////////////////////////////////////////

    if (formula->src_url == NULL) {
        resultCode = ppkg_fetch_git(packageName, formula, ppkgDownloadsDir, ppkgDownloadsDirLength);
    } else {
        if (formula->src_is_dir) {
            fprintf(stderr, "src_url is point to local dir, so no need to fetch.\n");
        } else {
            resultCode = ppkg_fetch_file(formula->src_url, formula->src_sha, ppkgDownloadsDir, ppkgDownloadsDirLength, verbose);
        }
    }

    if (resultCode != PPKG_OK) {
        goto clean;
    }

    if (formula->fix_url != NULL) {
        resultCode = ppkg_fetch_file(formula->fix_url, formula->fix_sha, ppkgDownloadsDir, ppkgDownloadsDirLength, verbose);

        if (resultCode != PPKG_OK) {
            goto clean;
        }
    }

    if (formula->res_url != NULL) {
        resultCode = ppkg_fetch_file(formula->res_url, formula->res_sha, ppkgDownloadsDir, ppkgDownloadsDirLength, verbose);

        if (resultCode != PPKG_OK) {
            goto clean;
        }
    }

clean:
    ppkg_formula_free(formula);
    return resultCode;
}
