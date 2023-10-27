#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>

#include "ppkg.h"

int ppkg_list_the_available_packages(PPKGPackageNameCallbak packageNameCallbak, const void * payload) {
    PPKGFormulaRepoList * formulaRepoList = NULL;

    int ret = ppkg_formula_repo_list(&formulaRepoList);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t j = 0;

    for (size_t i = 0; i < formulaRepoList->size; i++) {
        char * formulaRepoPath  = formulaRepoList->repos[i]->path;

        size_t formulaDIRLength = strlen(formulaRepoPath) + 10U;
        char   formulaDIR[formulaDIRLength];
        snprintf(formulaDIR, formulaDIRLength, "%s/formula", formulaRepoPath);

        struct stat status;

        if (stat(formulaDIR, &status) != 0) {
            continue;
        }

        if (!S_ISDIR(status.st_mode)) {
            continue;
        }

        DIR * dir = opendir(formulaDIR);

        if (dir == NULL) {
            perror(formulaDIR);
            ppkg_formula_repo_list_free(formulaRepoList);
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
                    break;
                } else {
                    perror(formulaDIR);
                    closedir(dir);
                    ppkg_formula_repo_list_free(formulaRepoList);
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

                    ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(fileName);

                    if (ret == PPKG_OK) {
                        ret = packageNameCallbak(fileName, j, payload);

                        j++;

                        if (ret != PPKG_OK) {
                            closedir(dir);
                            ppkg_formula_repo_list_free(formulaRepoList);
                            return ret;
                        }
                    }
                }
            }
        }
    }

    ppkg_formula_repo_list_free(formulaRepoList);

    return PPKG_OK;
}

static int package_name_callback(const char * packageName, size_t i, const void * payload) {
    printf("%s\n", packageName);
    return PPKG_OK;
}

int ppkg_show_the_available_packages() {
    return ppkg_list_the_available_packages(package_name_callback, NULL);
}
