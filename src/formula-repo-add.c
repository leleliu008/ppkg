#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#include "ppkg.h"

int ppkg_formula_repo_add(const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName) {
    if (formulaRepoName == NULL) {
        return PPKG_ARG_IS_NULL;
    }

    if (strcmp(formulaRepoName, "") == 0) {
        return PPKG_ARG_IS_EMPTY;
    }

    if (strcmp(formulaRepoName, "offical-core") == 0) {
        fprintf(stderr, "offical-core is reserved, please use other name.\n");
        return PPKG_ERROR;
    }

    if (formulaRepoUrl == NULL) {
        return PPKG_ARG_IS_NULL;
    }

    if (strcmp(formulaRepoUrl, "") == 0) {
        return PPKG_ARG_IS_EMPTY;
    }

    if (branchName == NULL || strcmp(branchName, "") == 0) {
        branchName = (char*)"master";
    }

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL || strcmp(userHomeDir, "") == 0) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    size_t  formulaRepoDBPathLength = userHomeDirLength + 16;
    char    formulaRepoDBPath[formulaRepoDBPathLength];
    memset (formulaRepoDBPath, 0, formulaRepoDBPathLength);
    sprintf(formulaRepoDBPath, "%s/.ppkg/repos.db", userHomeDir);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    sqlite3 * db = NULL;

    int resultCode = sqlite3_open(formulaRepoDBPath, &db);

    if (resultCode != SQLITE_OK) {
        fprintf(stderr, "%s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return resultCode;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const char * createTableSql = "CREATE TABLE IF NOT EXISTS formulaRepo (name TEXT PRIMARY KEY, url TEXT NOT NULL, branchName TEXT NOT NULL);";
    char * errorMsg = NULL;

    resultCode = sqlite3_exec(db, createTableSql, NULL, NULL, &errorMsg);

    if (resultCode != SQLITE_OK) {
        fprintf(stderr, "%s\n", errorMsg);
        sqlite3_close(db);
        return resultCode;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    size_t  updateSqlLength = 80 + strlen(formulaRepoName) + strlen(formulaRepoUrl) + strlen(branchName);
    char    updateSql[updateSqlLength];
    memset( updateSql, 0, updateSqlLength);
    sprintf(updateSql, "INSERT OR REPLACE INTO formulaRepo (name,url,branchName) VALUES ('%s','%s','%s');", formulaRepoName, formulaRepoUrl, branchName);

    errorMsg = NULL;

    resultCode = sqlite3_exec(db, updateSql, NULL, NULL, &errorMsg);

    if (resultCode != SQLITE_OK) {
        fprintf(stderr, "%s\n", errorMsg);
    }

    sqlite3_close(db);

    return resultCode;
}
