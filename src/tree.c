#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "ppkg.h"

int ppkg_tree(const char * packageName, size_t argc, char* argv[]) {
    int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    char   ppkgHomeDIR[256] = {0};
    size_t ppkgHomeDIRLength;

    ret = ppkg_home_dir(ppkgHomeDIR, 255, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t   packageInstalledDIRLength = ppkgHomeDIRLength + strlen(packageName) + 12U;
    char     packageInstalledDIR[packageInstalledDIRLength];
    snprintf(packageInstalledDIR, packageInstalledDIRLength, "%s/installed/%s", ppkgHomeDIR, packageName);

    struct stat st;

    if (stat(packageInstalledDIR, &st) != 0) {
        return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    size_t   receiptFilePathLength = packageInstalledDIRLength + 20U;
    char     receiptFilePath[receiptFilePathLength];
    snprintf(receiptFilePath, receiptFilePathLength, "%s/.ppkg/RECEIPT.yml", packageInstalledDIR);

    if (stat(receiptFilePath, &st) != 0 || (!S_ISREG(st.st_mode))) {
        return PPKG_ERROR_PACKAGE_IS_BROKEN;
    }

    size_t   treeCommandPathLength = ppkgHomeDIRLength + 15U;
    char     treeCommandPath[treeCommandPathLength];
    snprintf(treeCommandPath, treeCommandPathLength, "%s/core/bin/tree", ppkgHomeDIR);

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