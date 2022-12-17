#include <stdio.h>
#include <string.h>

#include "core/fs.h"
#include "core/rm-r.h"
#include "ppkg.h"

int ppkg_uninstall(const char * packageName, bool verbose) {
    int resultCode = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL || strcmp(userHomeDir, "") == 0) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    size_t  installedDirLength = userHomeDirLength + strlen(packageName) + 20;
    char    installedDir[installedDirLength];
    memset (installedDir, 0, installedDirLength);
    sprintf(installedDir, "%s/.ppkg/installed/%s", userHomeDir, packageName);

    size_t  receiptFilePathLength = installedDirLength + 20;
    char    receiptFilePath[receiptFilePathLength];
    memset (receiptFilePath, 0, receiptFilePathLength);
    sprintf(receiptFilePath, "%s/.ppkg/receipt.yml", installedDir);

    if (exists_and_is_a_regular_file(receiptFilePath)) {
        if (rm_r(installedDir, verbose) == 0) {
            return PPKG_OK;
        } else {
            return PPKG_ERROR;
        }
    }

    return PPKG_PACKAGE_IS_NOT_INSTALLED;
}
