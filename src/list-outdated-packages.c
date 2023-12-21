#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>

#include "ppkg.h"

int ppkg_list_the__outdated_packages(const PPKGTargetPlatform * targetPlatform, const bool verbose) {
    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    int ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    struct stat st;

    size_t ppkgInstalledRootDIRCapacity = ppkgHomeDIRLength + 15U + targetPlatform->nameLen + targetPlatform->versLen + targetPlatform->archLen;
    char   ppkgInstalledRootDIR[ppkgInstalledRootDIRCapacity];

    ret = snprintf(ppkgInstalledRootDIR, ppkgInstalledRootDIRCapacity, "%s/installed/%s-%s-%s", ppkgHomeDIR, targetPlatform->name, targetPlatform->vers, targetPlatform->arch);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (stat(ppkgInstalledRootDIR, &st) != 0 || (!S_ISDIR(st.st_mode))) {
        return PPKG_OK;
    }

    DIR * dir = opendir(ppkgInstalledRootDIR);

    if (dir == NULL) {
        perror(ppkgInstalledRootDIR);
        return PPKG_ERROR;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                break;
            } else {
                perror(ppkgInstalledRootDIR);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t packageInstalledDIRCapacity = ppkgInstalledRootDIRCapacity + strlen(dir_entry->d_name) + 2U;
        char   packageInstalledDIR[packageInstalledDIRCapacity];

        ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", ppkgInstalledRootDIR, dir_entry->d_name);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            return PPKG_ERROR;
        }

        if (lstat(packageInstalledDIR, &st) == 0) {
            if (!S_ISLNK(st.st_mode)) {
                continue;
            }
        } else {
            continue;
        }

        size_t receiptFilePathCapacity = packageInstalledDIRCapacity + 20U;
        char   receiptFilePath[receiptFilePathCapacity];

        ret = snprintf(receiptFilePath, receiptFilePathCapacity, "%s/.ppkg/RECEIPT.yml", packageInstalledDIR);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            return PPKG_ERROR;
        }

        if (lstat(receiptFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            //printf("%s\n", dir_entry->d_name);

            PPKGReceipt * receipt = NULL;

            ret = ppkg_receipt_parse(dir_entry->d_name, targetPlatform, &receipt);

            if (ret != PPKG_OK) {
                closedir(dir);
                ppkg_receipt_free(receipt);
                receipt = NULL;
                return ret;
            }

            PPKGFormula * formula = NULL;

            ret = ppkg_formula_lookup(dir_entry->d_name, targetPlatform->name, &formula);

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
