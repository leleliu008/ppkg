#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fnmatch.h>

#include "core/log.h"
#include "core/fs.h"
#include "ppkg.h"

int ppkg_search(const char * keyword) {
    if (keyword == NULL) {
        return PPKG_ARG_IS_NULL;
    }

    if (strcmp(keyword, "") == 0) {
        return PPKG_ARG_IS_EMPTY;
    }

    PPKGFormulaRepoList * formulaRepoList = NULL;

    int resultCode = ppkg_formula_repo_list_new(&formulaRepoList);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    bool isFirst = true;

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

            size_t  patternLength = strlen(keyword) + 7;
            char    pattern[patternLength];
            memset (pattern, 0, patternLength);
            sprintf(pattern, "*%s*.yml", keyword);

            int r = fnmatch(pattern, dir_entry->d_name, 0);

            if (r == 0) {
                size_t  fileNameLength = strlen(dir_entry->d_name);
                char    packageName[fileNameLength];
                memset (packageName, 0, fileNameLength);
                strncpy(packageName, dir_entry->d_name, fileNameLength - 4);

                if (isFirst) {
                    isFirst = false;
                } else {
                    printf("\n");
                }

                //printf("%s\n", packageName);
                resultCode = ppkg_info(packageName, NULL);

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
