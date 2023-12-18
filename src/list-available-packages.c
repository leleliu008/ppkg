#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>

#include "ppkg.h"

static int _list_dir(const char * formulaDIR, const char * targetPlatformName, PPKGPackageNameCallbak packageNameCallbak, const void * payload, size_t * counter) {
    DIR * dir = opendir(formulaDIR);

    if (dir == NULL) {
        perror(formulaDIR);
        return PPKG_ERROR;
    }

    char * fileName;
    char * fileNameSuffix;
    size_t fileNameLength;

    struct dirent * dir_entry;

    for (;;) {
        errno = 0;

        dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return PPKG_OK;
            } else {
                perror(formulaDIR);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        //puts(dir_entry->d_name);

        fileName = dir_entry->d_name;

        fileNameLength = strlen(fileName);

        if (fileNameLength > 4) {
            fileNameSuffix = fileName + fileNameLength - 4;

            if (strcmp(fileNameSuffix, ".yml") == 0) {
                fileName[fileNameLength - 4] = '\0';

                int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(fileName);

                if (ret == PPKG_OK) {
                    ret = packageNameCallbak(fileName, targetPlatformName, *counter, payload);

                    (*counter)++;

                    if (ret != PPKG_OK) {
                        closedir(dir);
                        return ret;
                    }
                }
            }
        }
    }
}

int ppkg_list_the_available_packages(const char * targetPlatformName, PPKGPackageNameCallbak packageNameCallbak, const void * payload) {
    PPKGFormulaRepoList * formulaRepoList = NULL;

    int ret = ppkg_formula_repo_list(&formulaRepoList);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t j = 0U;

    struct stat status;

    for (size_t i = 0U; i < formulaRepoList->size; i++) {
        char * formulaRepoPath  = formulaRepoList->repos[i]->path;

        if (targetPlatformName != NULL && targetPlatformName[0] != '\0') {
            size_t formulaDIRCapacity = strlen(formulaRepoPath) + strlen(targetPlatformName) + 10U;
            char   formulaDIR[formulaDIRCapacity];

            ret = snprintf(formulaDIR, formulaDIRCapacity, "%s/formula/%s", formulaRepoPath, targetPlatformName);

            if (ret < 0) {
                perror(NULL);
                ppkg_formula_repo_list_free(formulaRepoList);
                return PPKG_ERROR;
            }

            if (stat(formulaDIR, &status) == 0 && S_ISDIR(status.st_mode)) {
                ret = _list_dir(formulaDIR, targetPlatformName, packageNameCallbak, payload, &j);

                if (ret < 0) {
                    ppkg_formula_repo_list_free(formulaRepoList);
                    return ret;
                }
            }
        }

        size_t formulaDIRCapacity = strlen(formulaRepoPath) + 10U;
        char   formulaDIR[formulaDIRCapacity];

        ret = snprintf(formulaDIR, formulaDIRCapacity, "%s/formula", formulaRepoPath);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (stat(formulaDIR, &status) == 0 && S_ISDIR(status.st_mode)) {
            ret = _list_dir(formulaDIR, targetPlatformName, packageNameCallbak, payload, &j);

            if (ret < 0) {
                ppkg_formula_repo_list_free(formulaRepoList);
                return ret;
            }
        }
    }

    ppkg_formula_repo_list_free(formulaRepoList);

    return PPKG_OK;
}

static int package_name_callback(const char * packageName, const char * targetPlatformName, size_t i, const void * payload) {
    printf("%s\n", packageName);
    return PPKG_OK;
}

int ppkg_show_the_available_packages(const char * targetPlatformName) {
    return ppkg_list_the_available_packages(targetPlatformName, package_name_callback, NULL);
}
