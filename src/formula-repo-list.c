#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "core/zlib-flate.h"
#include "core/fs.h"
#include "ppkg.h"

int ppkg_formula_repo_list_new(PPKGFormulaRepoList * * out) {
    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t  ppkgFormulaRepoDirLength = userHomeDirLength + 15;
    char    ppkgFormulaRepoDir[ppkgFormulaRepoDirLength];
    memset (ppkgFormulaRepoDir, 0, ppkgFormulaRepoDirLength);
    snprintf(ppkgFormulaRepoDir, ppkgFormulaRepoDirLength, "%s/.ppkg/repos.d", userHomeDir);

    if (!exists_and_is_a_directory(ppkgFormulaRepoDir)) {
        PPKGFormulaRepoList* formulaRepoList = (PPKGFormulaRepoList*)calloc(1, sizeof(PPKGFormulaRepoList));

        if (formulaRepoList == NULL) {
            return PPKG_ERROR_ALLOCATE_MEMORY_FAILED;
        } else {
            (*out) = formulaRepoList;
            return PPKG_OK;
        }
    }

    DIR           * dir;
    struct dirent * dir_entry;

    dir = opendir(ppkgFormulaRepoDir);

    if (dir == NULL) {
        perror(ppkgFormulaRepoDir);
        return PPKG_ERROR;
    }

    size_t capcity = 5;

    PPKGFormulaRepoList * formulaRepoList = NULL;

    int resultCode = PPKG_OK;

    while ((dir_entry = readdir(dir))) {
        //puts(dir_entry->d_name);
        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t  formulaRepoPathLength = ppkgFormulaRepoDirLength + strlen(dir_entry->d_name) + 2;
        char    formulaRepoPath[formulaRepoPathLength];
        memset (formulaRepoPath, 0, formulaRepoPathLength);
        snprintf(formulaRepoPath, formulaRepoPathLength, "%s/%s", ppkgFormulaRepoDir, dir_entry->d_name);

        size_t  formulaRepoConfigFilePathLength = formulaRepoPathLength + 24;
        char    formulaRepoConfigFilePath[formulaRepoConfigFilePathLength];
        memset (formulaRepoConfigFilePath, 0, formulaRepoConfigFilePathLength);
        snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathLength, "%s/.ppkg-formula-repo.dat", formulaRepoPath);

        if (exists_and_is_a_regular_file(formulaRepoConfigFilePath)) {
            PPKGFormulaRepo * formulaRepo = NULL;

            size_t  formulaRepoConfigFilePath2Length = formulaRepoPathLength + 24;
            char    formulaRepoConfigFilePath2[formulaRepoConfigFilePath2Length];
            memset (formulaRepoConfigFilePath2, 0, formulaRepoConfigFilePath2Length);
            snprintf(formulaRepoConfigFilePath2, formulaRepoConfigFilePath2Length, "%s/.ppkg-formula-repo.yml", formulaRepoPath);

            FILE * inputFile  = NULL;
            FILE * outputFile = NULL;

            inputFile = fopen(formulaRepoConfigFilePath, "rb");

            if (inputFile == NULL) {
                perror(formulaRepoConfigFilePath);
                goto clean;
            }

            outputFile = fopen(formulaRepoConfigFilePath2, "w");

            if (outputFile == NULL) {
                perror(formulaRepoConfigFilePath2);
                fclose(inputFile);
                goto clean;
            }

            resultCode = zlib_inflate_file_to_file(inputFile, outputFile);

            fclose(inputFile);
            fclose(outputFile);

            if (resultCode != 0) {
                goto clean;
            }

            resultCode = ppkg_formula_repo_parse(formulaRepoConfigFilePath2, &formulaRepo);

            if (resultCode != PPKG_OK) {
                //parse failed, treat it as a invalid ppkg formula repo.
                continue;
            }

            if (formulaRepoList == NULL) {
                formulaRepoList = (PPKGFormulaRepoList*)calloc(1, sizeof(PPKGFormulaRepoList));

                if (formulaRepoList == NULL) {
                    ppkg_formula_repo_free(formulaRepo);
                    resultCode = PPKG_ERROR_ALLOCATE_MEMORY_FAILED;
                    goto clean;
                }

                formulaRepoList->repos = (PPKGFormulaRepo**)calloc(capcity, sizeof(PPKGFormulaRepo*));

                if (formulaRepoList->repos == NULL) {
                    ppkg_formula_repo_free(formulaRepo);
                    resultCode = PPKG_ERROR_ALLOCATE_MEMORY_FAILED;
                    goto clean;
                }
            }

            if (capcity == formulaRepoList->size) {
                capcity += 5;
                PPKGFormulaRepo ** formulaRepoArray = (PPKGFormulaRepo**)realloc(formulaRepoList->repos, capcity * sizeof(PPKGFormulaRepo*));

                if (formulaRepoArray == NULL) {
                    ppkg_formula_repo_free(formulaRepo);
                    ppkg_formula_repo_list_free(formulaRepoList);
                    resultCode = PPKG_ERROR_ALLOCATE_MEMORY_FAILED;
                    goto clean;
                } else {
                    formulaRepoList->repos = formulaRepoArray;
                }
            }

            formulaRepo->name = strdup(dir_entry->d_name);
            formulaRepo->path = strdup(formulaRepoPath);

            formulaRepoList->repos[formulaRepoList->size] = formulaRepo;
            formulaRepoList->size += 1;
        } else {
            continue;
        }
    }

    if (formulaRepoList == NULL) {
        formulaRepoList = (PPKGFormulaRepoList*)calloc(1, sizeof(PPKGFormulaRepoList));

        if (formulaRepoList == NULL) {
            resultCode = PPKG_ERROR_ALLOCATE_MEMORY_FAILED;
            goto clean;
        }
    }

clean:
    if (resultCode == PPKG_OK) {
        (*out) = formulaRepoList;
    } else {
        ppkg_formula_repo_list_free(formulaRepoList);
    }

    closedir(dir);

    return resultCode;
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
