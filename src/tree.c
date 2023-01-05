#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ppkg.h"

static int ppkg_tree_internal(char * packageInstalledDir, char * treeCommandPath, size_t argc, char* argv[]) {
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

    struct stat st;

    if (!((stat(receiptFilePath, &st) == 0) && S_ISREG(st.st_mode))) {
        return PPKG_PACKAGE_IS_NOT_INSTALLED;
    }

    /////////////////////////////////////////////////////////////////////////////////

    size_t  treeCommandPath1Length = userHomeDirLength + 31;
    char    treeCommandPath1[treeCommandPath1Length];
    memset (treeCommandPath1, 0, treeCommandPath1Length);
    sprintf(treeCommandPath1, "%s/.uppm/installed/tree/bin/tree", userHomeDir);

    struct stat st1;

    if ((stat(treeCommandPath1, &st1) == 0) && (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)) && (access(treeCommandPath1, X_OK) == 0)) {
        return ppkg_tree_internal(packageInstalledDir, treeCommandPath1, argc, argv);
    }

    /////////////////////////////////////////////////////////////////////////////////

    size_t  treeCommandPath2Length = userHomeDirLength + 31;
    char    treeCommandPath2[treeCommandPath2Length];
    memset (treeCommandPath2, 0, treeCommandPath2Length);
    sprintf(treeCommandPath2, "%s/.ppkg/installed/tree/bin/tree", userHomeDir);

    struct stat st2;

    if ((stat(treeCommandPath1, &st2) == 0) && (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)) && (access(treeCommandPath2, X_OK) == 0)) {
        return ppkg_tree_internal(packageInstalledDir, treeCommandPath2, argc, argv);
    }

    fprintf(stderr, "please install tree package first via ppkg install tree, then try again.\n");
    return PPKG_ERROR;
}
