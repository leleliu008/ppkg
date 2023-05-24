#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

#include "ppkg.h"

int ppkg_list_the_outdated__packages() {
    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t installedDirLength = userHomeDirLength + 17U;
    char   installedDir[installedDirLength];
    snprintf(installedDir, installedDirLength, "%s/.ppkg/installed", userHomeDir);

    struct stat st;

    if (stat(installedDir, &st) != 0 || (!S_ISDIR(st.st_mode))) {
        return PPKG_OK;
    }

    DIR * dir = opendir(installedDir);

    if (dir == NULL) {
        perror(installedDir);
        return PPKG_ERROR;
    }

    struct dirent * dir_entry;

    for (;;) {
        errno = 0;

        dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                break;
            } else {
                perror(installedDir);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t receiptFilePathLength = installedDirLength + strlen(dir_entry->d_name) + 20U;
        char   receiptFilePath[receiptFilePathLength];
        snprintf(receiptFilePath, receiptFilePathLength, "%s/%s/.ppkg/receipt.yml", installedDir, dir_entry->d_name);

        if (stat(receiptFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            //printf("%s\n", dir_entry->d_name);

            PPKGReceipt * receipt = NULL;

            int ret = ppkg_receipt_parse(dir_entry->d_name, &receipt);

            if (ret != PPKG_OK) {
                closedir(dir);
                ppkg_receipt_free(receipt);
                receipt = NULL;
                return ret;
            }

            PPKGFormula * formula = NULL;

            ret = ppkg_formula_lookup(dir_entry->d_name, &formula);

            if (ret == PPKG_OK) {
                if (strcmp(receipt->version, formula->version) != 0) {
                    printf("%s %s => %s\n", dir_entry->d_name, receipt->version, formula->version);
                }
            }

            ppkg_formula_free(formula);
            ppkg_receipt_free(receipt);

            formula = NULL;
            receipt = NULL;
        }
    }

    return PPKG_OK;
}
