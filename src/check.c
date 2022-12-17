#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "core/fs.h"
#include "core/util.h"
#include "core/regex/regex.h"
#include "ppkg.h"

int ppkg_check_if_the_given_argument_matches_package_name_pattern(const char * arg) {
    if (arg == NULL) {
        return PPKG_ARG_IS_NULL;
    } else if (strcmp(arg, "") == 0) {
        return PPKG_ARG_IS_EMPTY;
    } else if (regex_matched(arg, PPKG_PACKAGE_NAME_PATTERN)) {
        return PPKG_OK;
    } else {
        return PPKG_ARG_IS_INVALID;
    }
}

int ppkg_check_if_the_given_package_is_available(const char * packageName) {
    int resultCode = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    PPKGFormulaRepoList * formulaRepoList = NULL;

    resultCode = ppkg_formula_repo_list_new(&formulaRepoList);

    if (resultCode != PPKG_OK) {
        ppkg_formula_repo_list_free(formulaRepoList);
        return resultCode;
    }

    for (size_t i = 0; i < formulaRepoList->size; i++) {
        char *  formulaRepoPath = formulaRepoList->repos[i]->path;
        size_t  formulaFilePathLength = strlen(formulaRepoPath) + strlen(packageName) + 15;
        char    formulaFilePath[formulaFilePathLength];
        memset (formulaFilePath, 0, formulaFilePathLength);
        sprintf(formulaFilePath, "%s/formula/%s.yml", formulaRepoPath, packageName);

        if (exists_and_is_a_regular_file(formulaFilePath)) {
            ppkg_formula_repo_list_free(formulaRepoList);
            return PPKG_OK;
        }
    }

    ppkg_formula_repo_list_free(formulaRepoList);
    return PPKG_PACKAGE_IS_NOT_AVAILABLE;
}

int ppkg_check_if_the_given_package_is_installed(const char * packageName) {
    int resultCode = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL || strcmp(userHomeDir, "") == 0) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    size_t  ppkgHomeDirLength = userHomeDirLength + 7; 
    char    ppkgHomeDir[ppkgHomeDirLength];
    memset (ppkgHomeDir, 0, ppkgHomeDirLength);
    sprintf(ppkgHomeDir, "%s/.ppkg", userHomeDir);

    if (!exists_and_is_a_directory(ppkgHomeDir)) {
        return PPKG_PACKAGE_IS_NOT_INSTALLED;
    }

    size_t  receiptFilePathLength = ppkgHomeDirLength + strlen(packageName) + 30;
    char    receiptFilePath[receiptFilePathLength];
    memset (receiptFilePath, 0, receiptFilePathLength);
    sprintf(receiptFilePath, "%s/installed/%s/.ppkg/receipt.yml", ppkgHomeDir, packageName);

    if (exists_and_is_a_regular_file(receiptFilePath)) {
        return PPKG_OK;
    } else {
        return PPKG_PACKAGE_IS_NOT_INSTALLED;
    }
}

int ppkg_check_if_the_given_package_is_outdated(const char * packageName) {
    PPKGFormula * formula = NULL;
    PPKGReceipt * receipt = NULL;

    int resultCode = ppkg_formula_parse(packageName, &formula);

    if (resultCode != PPKG_OK) {
        goto clean;
    }

    resultCode = ppkg_receipt_parse(packageName, &receipt);

    if (resultCode != PPKG_OK) {
        goto clean;
    }

    if (strcmp(receipt->version, formula->version) == 0) {
        resultCode = PPKG_PACKAGE_IS_NOT_OUTDATED;
    }

clean:
    ppkg_formula_free(formula);
    ppkg_receipt_free(receipt);
    return resultCode;
}
