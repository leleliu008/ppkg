#include <stdio.h>
#include <string.h>

#include "core/fs.h"
#include "ppkg.h"

#include <sqlite3.h>

PPKGFormulaRepo* ppkg_formula_repo_default_new(char * userHomeDir, size_t userHomeDirLength) {
    char *  formulaRepoPath = (char*)calloc(userHomeDirLength + 30, sizeof(char));

    if (formulaRepoPath == NULL) {
        return NULL;
    }

    sprintf(formulaRepoPath, "%s/.ppkg/repos.d/offical-core", userHomeDir);

    PPKGFormulaRepo * formulaRepo = (PPKGFormulaRepo*)calloc(1, sizeof(PPKGFormulaRepo));

    if (formulaRepo == NULL) {
        return NULL;
    }

    formulaRepo->branch = strdup("master");
    formulaRepo->name   = strdup("offical-core");
    formulaRepo->url    = strdup("https://github.com/leleliu008/ppkg-formula-repository-offical-core");
    formulaRepo->path   = formulaRepoPath;

    return formulaRepo;
}

int ppkg_formula_repo_list_new(PPKGFormulaRepoList * * out) {
    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL || strcmp(userHomeDir, "") == 0) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    size_t  formulaRepoDBPathLength = userHomeDirLength + 16;
    char    formulaRepoDBPath[formulaRepoDBPathLength];
    memset (formulaRepoDBPath, 0, formulaRepoDBPathLength);
    sprintf(formulaRepoDBPath, "%s/.ppkg/repos.db", userHomeDir);

    PPKGFormulaRepoList * formulaRepoList = (PPKGFormulaRepoList*)calloc(1, sizeof(PPKGFormulaRepoList));
    formulaRepoList->repos = (PPKGFormulaRepo**)calloc(1, sizeof(PPKGFormulaRepo*));
    formulaRepoList->repos[0] = ppkg_formula_repo_default_new(userHomeDir, userHomeDirLength);
    formulaRepoList->size     = 1;

    if (!exists_and_is_a_regular_file(formulaRepoDBPath)) {
        (*out) = formulaRepoList;
        return PPKG_OK;
    }

    size_t capacity = 5;

    sqlite3      * db        = NULL;
    sqlite3_stmt * statement = NULL;

    int resultCode = sqlite3_open(formulaRepoDBPath, &db);

    if (resultCode != SQLITE_OK) {
        goto clean;
    }

    resultCode = sqlite3_prepare_v2(db, "select * from formulaRepo", -1, &statement, NULL);

    if (resultCode != SQLITE_OK) {
        if (strcmp(sqlite3_errmsg(db), "Error: no such table: formulaRepo") == 0) {
            resultCode = SQLITE_OK;
        }
        goto clean;
    }

    for(;;) {
        resultCode = sqlite3_step(statement);

        if (resultCode == SQLITE_ROW) {
            resultCode =  SQLITE_OK;

            char * formulaRepoName   = (char *)sqlite3_column_text(statement, 0);
            char * formulaRepoUrl    = (char *)sqlite3_column_text(statement, 1);
            char * formulaRepoBranch = (char *)sqlite3_column_text(statement, 2);

            //printf("formulaRepoName=%s\nformulaRepoUrl=%s\nformulaRepoBranch=%s\n", formulaRepoName, formulaRepoUrl, formulaRepoBranch);

            char *  formulaRepoPath = (char*)calloc(userHomeDirLength + 16 + strlen(formulaRepoName), sizeof(char));
            sprintf(formulaRepoPath, "%s/.ppkg/repos.d/%s", userHomeDir, formulaRepoName);

            PPKGFormulaRepo * formulaRepo = (PPKGFormulaRepo*)calloc(1, sizeof(PPKGFormulaRepo));
            formulaRepo->name = strdup(formulaRepoName);
            formulaRepo->url  = strdup(formulaRepoUrl);
            formulaRepo->path =        formulaRepoPath;
            formulaRepo->branch  = strdup(formulaRepoBranch);

            if (formulaRepoList->size == capacity) {
                capacity += capacity;
                formulaRepoList->repos = (PPKGFormulaRepo**)realloc(formulaRepoList->repos, capacity * sizeof(PPKGFormulaRepo*));
            }

            formulaRepoList->repos[formulaRepoList->size] = formulaRepo;
            formulaRepoList->size += 1;

            continue;
        }

        if (resultCode == SQLITE_DONE) {
            resultCode =  SQLITE_OK;
        }

        break;
    }

clean:
    if (resultCode == SQLITE_OK) {
        if (formulaRepoList == NULL) {
            formulaRepoList = (PPKGFormulaRepoList*)calloc(1, sizeof(PPKGFormulaRepoList));
            formulaRepoList->repos = (PPKGFormulaRepo**)calloc(1, sizeof(PPKGFormulaRepo*));
            formulaRepoList->repos[0] = ppkg_formula_repo_default_new(userHomeDir, userHomeDirLength);
            formulaRepoList->size     = 1;
        }

        (*out) = formulaRepoList;
    } else {
        fprintf(stderr, "%s\n", sqlite3_errmsg(db));
    }

    if (statement != NULL) {
        sqlite3_finalize(statement);
    }

    if (db != NULL) {
        sqlite3_close(db);
    }

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

        free(formulaRepo->name);
        free(formulaRepo->url);
        free(formulaRepo->path);
        free(formulaRepo->branch);

        formulaRepo->name = NULL;
        formulaRepo->url  = NULL;
        formulaRepo->path = NULL;
        formulaRepo->branch = NULL;

        free(formulaRepoList->repos[i]);
        formulaRepoList->repos[i] = NULL;
    }

    free(formulaRepoList->repos);
    formulaRepoList->repos = NULL;

    free(formulaRepoList);
}
