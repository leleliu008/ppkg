#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>
#include <sys/stat.h>

#include "core/fs.h"
#include "core/http.h"
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
        sprintf(formulaDir, "%s/formula", formulaRepoPath);

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

    PPKGFormula * formula = NULL;

    int resultCode = ppkg_formula_parse(packageName, &formula);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    size_t urlLength = strlen(formula->src_url);
    size_t urlCopyLength = urlLength + 1;
    char   urlCopy[urlCopyLength];
    memset(urlCopy, 0, urlCopyLength);
    strcpy(urlCopy, formula->src_url);

    const char * archiveFileName = basename(urlCopy);

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL || strcmp(userHomeDir, "") == 0) {
        ppkg_formula_free(formula);
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    size_t  downloadDirLength = userHomeDirLength + 18;
    char    downloadDir[downloadDirLength];
    memset (downloadDir, 0, downloadDirLength);
    sprintf(downloadDir, "%s/.ppkg/downloads", userHomeDir);

    if (!exists_and_is_a_directory(downloadDir)) {
        if (mkdir(downloadDir, S_IRWXU) != 0) {
            perror(downloadDir);
            ppkg_formula_free(formula);
            return PPKG_ERROR;
        }
    }

    size_t  archiveFilePathLength = downloadDirLength + strlen(archiveFileName) + 2;
    char    archiveFilePath[archiveFilePathLength];
    memset (archiveFilePath, 0, archiveFilePathLength);
    sprintf(archiveFilePath, "%s/%s", downloadDir, archiveFileName);

    if (exists_and_is_a_regular_file(archiveFilePath)) {
        char * actualSHA256SUM = sha256sum_of_file(archiveFilePath);
        if (strcmp(actualSHA256SUM, formula->src_sha) == 0) {
            free(actualSHA256SUM);
            ppkg_formula_free(formula);
            fprintf(stderr, "%s already have been fetched.\n", archiveFilePath);
            return PPKG_OK;
        } else {
            free(actualSHA256SUM);
        }
    }

    if (http_fetch_to_file(formula->src_url, archiveFilePath, verbose, verbose) != 0) {
        ppkg_formula_free(formula);
        return PPKG_NETWORK_ERROR;
    }

    char * actualSHA256SUM = sha256sum_of_file(archiveFilePath);

    if (strcmp(actualSHA256SUM, formula->src_sha) == 0) {
        free(actualSHA256SUM);
        ppkg_formula_free(formula);
        return PPKG_OK;
    } else {
        fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", formula->src_sha, actualSHA256SUM);
        free(actualSHA256SUM);
        ppkg_formula_free(formula);
        return PPKG_SHA256_MISMATCH;
    }
}
