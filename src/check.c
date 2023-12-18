#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <limits.h>
#include <sys/stat.h>

#include "core/regex/regex.h"

#include "ppkg.h"

int ppkg_check_if_the_given_argument_matches_package_name_pattern(const char * arg) {
    if (arg == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (arg[0] == '\0') {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    if (regex_matched(arg, PPKG_PACKAGE_NAME_PATTERN) == 0) {
        return PPKG_OK;
    } else {
        if (errno == 0) {
            return PPKG_ERROR_ARG_IS_INVALID;
        } else {
            perror(NULL);
            return PPKG_ERROR;
        }
    }
}

int ppkg_check_if_the_given_package_is_available(const char * packageName, const char * targetPlatformName) {
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

        if (targetPlatformName != NULL && targetPlatformName[0] != '\0') {
            size_t formulaFilePathLength = strlen(formulaRepoPath) + strlen(targetPlatformName) + strlen(packageName) + 15U;
            char   formulaFilePath[formulaFilePathLength];

            ret = snprintf(formulaFilePath, formulaFilePathLength, "%s/formula/%s/%s.yml", formulaRepoPath, targetPlatformName, packageName);

            if (ret < 0) {
                perror(NULL);
                ppkg_formula_repo_list_free(formulaRepoList);
                return PPKG_ERROR;
            }

            if (lstat(formulaFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
                ppkg_formula_repo_list_free(formulaRepoList);
                return PPKG_OK;
            }
        }

        size_t formulaFilePathLength = strlen(formulaRepoPath) + strlen(packageName) + 15U;
        char   formulaFilePath[formulaFilePathLength];

        ret = snprintf(formulaFilePath, formulaFilePathLength, "%s/formula/%s.yml", formulaRepoPath, packageName);

        if (ret < 0) {
            perror(NULL);
            ppkg_formula_repo_list_free(formulaRepoList);
            return PPKG_ERROR;
        }

        if (lstat(formulaFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            ppkg_formula_repo_list_free(formulaRepoList);
            return PPKG_OK;
        }
    }

    ppkg_formula_repo_list_free(formulaRepoList);
    return PPKG_ERROR_PACKAGE_NOT_AVAILABLE;
}

int ppkg_check_if_the_given_package_is_installed(const char * packageName, const PPKGTargetPlatform * targetPlatform) {
    int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    struct stat st;

    size_t packageInstalledDIRCapacity = ppkgHomeDIRLength + strlen(targetPlatform->name) + strlen(targetPlatform->version) + strlen(targetPlatform->arch) + strlen(packageName) + 15U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/installed/%s-%s-%s/%s", ppkgHomeDIR, targetPlatform->name, targetPlatform->version, targetPlatform->arch, packageName);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (lstat(packageInstalledDIR, &st) == 0) {
        if (!S_ISLNK(st.st_mode)) {
            return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
        }
    } else {
        return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    size_t receiptFilePathCapacity = packageInstalledDIRCapacity + 19U;
    char   receiptFilePath[receiptFilePathCapacity];

    ret = snprintf(receiptFilePath, receiptFilePathCapacity, "%s/.ppkg/RECEIPT.yml", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (lstat(receiptFilePath, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            return PPKG_OK;
        } else {
            return PPKG_ERROR_PACKAGE_IS_BROKEN;
        }
    } else {
        return PPKG_ERROR_PACKAGE_IS_BROKEN;
    }
}

int ppkg_check_if_the_given_package_is_outdated(const char * packageName, const PPKGTargetPlatform * targetPlatform) {
    PPKGFormula * formula = NULL;
    PPKGReceipt * receipt = NULL;

    int ret = ppkg_formula_lookup(packageName, targetPlatform->name, &formula);

    if (ret != PPKG_OK) {
        goto finalize;
    }

    ret = ppkg_receipt_parse(packageName, targetPlatform, &receipt);

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
