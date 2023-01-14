#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fnmatch.h>

#include "core/log.h"
#include "core/fs.h"
#include "ppkg.h"

int ppkg_list_the_outdated__packages() {
    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t  installedDirLength = userHomeDirLength + 17; 
    char    installedDir[installedDirLength];
    memset (installedDir, 0, installedDirLength);
    snprintf(installedDir, userHomeDirLength, "%s/.ppkg/installed", userHomeDir);

    if (!exists_and_is_a_directory(installedDir)) {
        return PPKG_OK;
    }

    DIR           * dir;
    struct dirent * dir_entry;

    dir = opendir(installedDir);

    if (dir == NULL) {
        perror(installedDir);
        return PPKG_ERROR;
    }

    while ((dir_entry = readdir(dir))) {
        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t  receiptFilePathLength = installedDirLength + strlen(dir_entry->d_name) + 20;
        char    receiptFilePath[receiptFilePathLength];
        memset (receiptFilePath, 0, receiptFilePathLength);
        snprintf(receiptFilePath, receiptFilePathLength, "%s/%s/.ppkg/receipt.yml", installedDir, dir_entry->d_name);

        if (exists_and_is_a_regular_file(receiptFilePath)) {
            //printf("%s\n", dir_entry->d_name);

            PPKGReceipt * receipt = NULL;

            int resultCode = ppkg_receipt_parse(dir_entry->d_name, &receipt);

            if (resultCode != PPKG_OK) {
                closedir(dir);
                ppkg_receipt_free(receipt);
                receipt = NULL;
                return resultCode;
            }

            PPKGFormula * formula = NULL;

            resultCode = ppkg_formula_parse(dir_entry->d_name, &formula);

            if (resultCode == PPKG_OK) {
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

    closedir(dir);

    return PPKG_OK;
}
