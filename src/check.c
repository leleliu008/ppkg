#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "core/regex/regex.h"
#include "ppkg.h"

int ppkg_check_if_the_given_argument_matches_package_name_pattern(const char * arg) {
    if (arg == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    } else if (arg[0] == '\0') {
        return PPKG_ERROR_ARG_IS_EMPTY;
    } else if (regex_matched(arg, PPKG_PACKAGE_NAME_PATTERN)) {
        return PPKG_OK;
    } else {
        return PPKG_ERROR_ARG_IS_INVALID;
    }
}

int ppkg_check_if_the_given_package_is_available(const char * packageName) {
    int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    PPKGFormulaRepoList * formulaRepoList = NULL;

    ret = ppkg_formula_repo_list(&formulaRepoList);

    if (ret != PPKG_OK) {
        ppkg_formula_repo_list_free(formulaRepoList);
        return ret;
    }

    struct stat st;

    for (size_t i = 0; i < formulaRepoList->size; i++) {
        char * formulaRepoPath = formulaRepoList->repos[i]->path;

        size_t formulaFilePathLength = strlen(formulaRepoPath) + strlen(packageName) + 15U;
        char   formulaFilePath[formulaFilePathLength];
        snprintf(formulaFilePath, formulaFilePathLength, "%s/formula/%s.yml", formulaRepoPath, packageName);

        if (stat(formulaFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            ppkg_formula_repo_list_free(formulaRepoList);
            return PPKG_OK;
        }
    }

    ppkg_formula_repo_list_free(formulaRepoList);
    return PPKG_ERROR_PACKAGE_NOT_AVAILABLE;
}

int ppkg_check_if_the_given_package_is_installed(const char * packageName) {
    int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    struct stat st;

    size_t ppkgHomeDirLength = userHomeDirLength + 7U;
    char   ppkgHomeDir[ppkgHomeDirLength];
    snprintf(ppkgHomeDir, ppkgHomeDirLength, "%s/.ppkg", userHomeDir);

    size_t packageInstalledDirLength = userHomeDirLength + strlen(packageName) + 20U;
    char   packageInstalledDir[packageInstalledDirLength];
    snprintf(packageInstalledDir, packageInstalledDirLength, "%s/.ppkg/installed/%s", userHomeDir, packageName);

    if (stat(packageInstalledDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", packageInstalledDir);
            return PPKG_ERROR;
        }
    } else {
        return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    size_t receiptFilePathLength = ppkgHomeDirLength + packageInstalledDirLength + 19U;
    char   receiptFilePath[receiptFilePathLength];
    snprintf(receiptFilePath, receiptFilePathLength, "%s/.ppkg/receipt.yml", packageInstalledDir);

    if (stat(receiptFilePath, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            return PPKG_OK;
        } else {
            return PPKG_ERROR_PACKAGE_IS_BROKEN;
        }
    } else {
        return PPKG_ERROR_PACKAGE_IS_BROKEN;
    }
}

int ppkg_check_if_the_given_package_is_outdated(const char * packageName) {
    PPKGFormula * formula = NULL;
    PPKGReceipt * receipt = NULL;

    int ret = ppkg_formula_lookup(packageName, &formula);

    if (ret != PPKG_OK) {
        goto finalize;
    }

    ret = ppkg_receipt_parse(packageName, &receipt);

    if (ret != PPKG_OK) {
        goto finalize;
    }

    if (strcmp(receipt->version, formula->version) == 0) {
        ret = PPKG_ERROR_PACKAGE_NOT_OUTDATED;
    }

finalize:
    ppkg_formula_free(formula);
    ppkg_receipt_free(receipt);
    return ret;
}
