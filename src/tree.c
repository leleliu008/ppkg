#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>

#include "ppkg.h"
#include "uppm.h"

int ppkg_tree(const char * packageName, const PPKGTargetPlatform * targetPlatform, size_t argc, char* argv[]) {
    int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    if (targetPlatform == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t packageInstalledDIRCapacity = ppkgHomeDIRLength + targetPlatform->nameLen + targetPlatform->versLen + targetPlatform->archLen + strlen(packageName) + 16U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/installed/%s-%s-%s/%s", ppkgHomeDIR, targetPlatform->name, targetPlatform->vers, targetPlatform->arch, packageName);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    struct stat st;

    if (stat(packageInstalledDIR, &st) != 0) {
        return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    size_t receiptFilePathLength = packageInstalledDIRCapacity + 20U;
    char   receiptFilePath[receiptFilePathLength];

    ret = snprintf(receiptFilePath, receiptFilePathLength, "%s/.ppkg/RECEIPT.yml", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (stat(receiptFilePath, &st) != 0 || (!S_ISREG(st.st_mode))) {
        return PPKG_ERROR_PACKAGE_IS_BROKEN;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t treeCommandPathCapacity = ppkgHomeDIRLength + 30U;
    char   treeCommandPath[treeCommandPathCapacity];

    ret = snprintf(treeCommandPath, treeCommandPathCapacity, "%s/uppm/installed/tree/bin/tree", ppkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (stat(treeCommandPath, &st) == 0) {
        if (!S_ISREG(st.st_mode)) {
            fprintf(stderr, "%s was expected to be a regular file, but it was not.\n", treeCommandPath);
            return PPKG_ERROR;
        }
    } else {
        ret = uppm_formula_repo_sync_official_core();

        if (ret != PPKG_OK) {
            return ret;
        }

        ret = uppm_install("tree", false, false);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t n = argc + 5U;
    char*  p[n];

    p[0] = treeCommandPath;
    p[1] = (char*)"--dirsfirst";
    p[2] = (char*)"-a";

    for (size_t i = 0U; i < argc; i++) {
        p[3U + i] = argv[i];
    }

    p[n - 2U] = packageInstalledDIR;
    p[n - 1U]   = NULL;

    execv(treeCommandPath, p);

    perror(treeCommandPath);

    return PPKG_ERROR;
}
