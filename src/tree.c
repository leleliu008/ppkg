#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "core/fs.h"
#include "ppkg.h"

int ppkg_tree(const char * packageName, size_t argc, char* argv[]) {
    int resultCode = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t  ppkgHomeDirLength = userHomeDirLength + 7;
    char    ppkgHomeDir[ppkgHomeDirLength];
    memset (ppkgHomeDir, 0, ppkgHomeDirLength);
    sprintf(ppkgHomeDir, "%s/.ppkg", userHomeDir);

    size_t  packageInstalledDirLength = ppkgHomeDirLength + strlen(packageName) + 12;
    char    packageInstalledDir[packageInstalledDirLength];
    memset (packageInstalledDir, 0, packageInstalledDirLength);
    sprintf(packageInstalledDir, "%s/installed/%s", ppkgHomeDir, packageName);

    size_t  receiptFilePathLength = packageInstalledDirLength + 20;
    char    receiptFilePath[receiptFilePathLength];
    memset (receiptFilePath, 0, receiptFilePathLength);
    sprintf(receiptFilePath, "%s/.ppkg/receipt.yml", packageInstalledDir);

    if (!exists_and_is_a_regular_file(receiptFilePath)) {
        return PPKG_PACKAGE_IS_NOT_INSTALLED;
    }

    size_t  treeCommandPathLength = strlen(userHomeDir) + 31;
    char    treeCommandPath[treeCommandPathLength];
    memset (treeCommandPath, 0, treeCommandPathLength);
    sprintf(treeCommandPath, "%s/.uppm/installed/tree/bin/tree", userHomeDir);

    size_t n = argc + 5;
    char*  p[n];

    p[0] = treeCommandPath;
    p[1] = (char*)"--dirsfirst";
    p[2] = (char*)"-a";

    for (size_t i = 0; i < argc; i++) {
        p[3+i] = argv[i];
    }

    p[n-2] = packageInstalledDir;
    p[n-1]   = NULL;

    if (execv(treeCommandPath, p) == -1) {
        perror(treeCommandPath);
        return PPKG_ERROR;
    } else {
        return PPKG_OK;
    }
}
