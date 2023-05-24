#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "core/rm-r.h"
#include "ppkg.h"

int ppkg_uninstall(const char * packageName, bool verbose) {
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

    size_t packageInstalledDirLength = userHomeDirLength + strlen(packageName) + 20U;
    char   packageInstalledDir[packageInstalledDirLength];
    snprintf(packageInstalledDir, packageInstalledDirLength, "%s/.ppkg/installed/%s", userHomeDir, packageName);

    struct stat st;

    if (stat(packageInstalledDir, &st) != 0) {
        return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    size_t receiptFilePathLength = packageInstalledDirLength + 20U;
    char   receiptFilePath[receiptFilePathLength];
    snprintf(receiptFilePath, receiptFilePathLength, "%s/.ppkg/receipt.yml", packageInstalledDir);

    if (stat(receiptFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
        return rm_r(packageInstalledDir, verbose);
    } else {
        // package is broken. is not installed completely?
        return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }
}
