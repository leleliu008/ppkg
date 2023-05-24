#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

#include "ppkg.h"

int ppkg_formula_repo_list(PPKGFormulaRepoList * * out) {
    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t ppkgFormulaRepoDirLength = userHomeDirLength + 15U;
    char   ppkgFormulaRepoDir[ppkgFormulaRepoDirLength];
    snprintf(ppkgFormulaRepoDir, ppkgFormulaRepoDirLength, "%s/.ppkg/repos.d", userHomeDir);

    struct stat st;

    if ((stat(ppkgFormulaRepoDir, &st) != 0) || (!S_ISDIR(st.st_mode))) {
        PPKGFormulaRepoList* formulaRepoList = (PPKGFormulaRepoList*)calloc(1, sizeof(PPKGFormulaRepoList));

        if (formulaRepoList == NULL) {
            return PPKG_ERROR_MEMORY_ALLOCATE;
        } else {
            (*out) = formulaRepoList;
            return PPKG_OK;
        }
    }

    DIR * dir = opendir(ppkgFormulaRepoDir);

    if (dir == NULL) {
        perror(ppkgFormulaRepoDir);
        return PPKG_ERROR;
    }

    size_t capcity = 5;

    PPKGFormulaRepoList * formulaRepoList = NULL;

    int ret = PPKG_OK;

    struct dirent * dir_entry;

    for (;;) {
        errno = 0;

        dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                break;
            } else {
                perror(ppkgFormulaRepoDir);
                closedir(dir);
                ppkg_formula_repo_list_free(formulaRepoList);
                return PPKG_ERROR;
            }
        }

        //puts(dir_entry->d_name);

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t formulaRepoPathLength = ppkgFormulaRepoDirLength + strlen(dir_entry->d_name) + 2U;
        char   formulaRepoPath[formulaRepoPathLength];
        snprintf(formulaRepoPath, formulaRepoPathLength, "%s/%s", ppkgFormulaRepoDir, dir_entry->d_name);

        size_t formulaRepoConfigFilePathLength = formulaRepoPathLength + 24U;
        char   formulaRepoConfigFilePath[formulaRepoConfigFilePathLength];
        snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathLength, "%s/.ppkg-formula-repo.yml", formulaRepoPath);

        if (stat(formulaRepoConfigFilePath, &st) != 0) {
            continue;
        }

        PPKGFormulaRepo * formulaRepo = NULL;

        ret = ppkg_formula_repo_parse(formulaRepoConfigFilePath, &formulaRepo);

        if (ret != PPKG_OK) {
            ppkg_formula_repo_free(formulaRepo);
            ppkg_formula_repo_list_free(formulaRepoList);
            goto finalize;
        }

        if (formulaRepoList == NULL) {
            formulaRepoList = (PPKGFormulaRepoList*)calloc(1, sizeof(PPKGFormulaRepoList));

            if (formulaRepoList == NULL) {
                ppkg_formula_repo_free(formulaRepo);
                ppkg_formula_repo_list_free(formulaRepoList);
                ret = PPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            formulaRepoList->repos = (PPKGFormulaRepo**)calloc(capcity, sizeof(PPKGFormulaRepo*));

            if (formulaRepoList->repos == NULL) {
                ppkg_formula_repo_free(formulaRepo);
                ppkg_formula_repo_list_free(formulaRepoList);
                ret = PPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }
        }

        if (capcity == formulaRepoList->size) {
            capcity += 5;
            PPKGFormulaRepo ** formulaRepoArray = (PPKGFormulaRepo**)realloc(formulaRepoList->repos, capcity * sizeof(PPKGFormulaRepo*));

            if (formulaRepoArray == NULL) {
                ppkg_formula_repo_free(formulaRepo);
                ppkg_formula_repo_list_free(formulaRepoList);
                ret = PPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            } else {
                formulaRepoList->repos = formulaRepoArray;
            }
        }

        formulaRepo->name = strdup(dir_entry->d_name);

        if (formulaRepo->name == NULL) {
            ppkg_formula_repo_free(formulaRepo);
            ppkg_formula_repo_list_free(formulaRepoList);
            ret = PPKG_ERROR_MEMORY_ALLOCATE;
            goto finalize;
        }

        formulaRepo->path = strdup(formulaRepoPath);

        if (formulaRepo->path == NULL) {
            ppkg_formula_repo_free(formulaRepo);
            ppkg_formula_repo_list_free(formulaRepoList);
            ret = PPKG_ERROR_MEMORY_ALLOCATE;
            goto finalize;
        }

        formulaRepoList->repos[formulaRepoList->size] = formulaRepo;
        formulaRepoList->size += 1;
    }

    if (formulaRepoList == NULL) {
        formulaRepoList = (PPKGFormulaRepoList*)calloc(1, sizeof(PPKGFormulaRepoList));

        if (formulaRepoList == NULL) {
            ret = PPKG_ERROR_MEMORY_ALLOCATE;
            goto finalize;
        }
    }

finalize:
    if (ret == PPKG_OK) {
        (*out) = formulaRepoList;
    } else {
        ppkg_formula_repo_list_free(formulaRepoList);
    }

    closedir(dir);

    return ret;
}

void ppkg_formula_repo_list_free(PPKGFormulaRepoList * formulaRepoList) {
    if (formulaRepoList == NULL) {
        return;
    }

    if (formulaRepoList->repos == NULL) {
        free(formulaRepoList);
        return;
    }

    for (size_t i = 0; i < formulaRepoList->size; i++) {
        PPKGFormulaRepo * formulaRepo = formulaRepoList->repos[i];
        ppkg_formula_repo_free(formulaRepo);
    }

    free(formulaRepoList->repos);
    formulaRepoList->repos = NULL;

    free(formulaRepoList);
}
