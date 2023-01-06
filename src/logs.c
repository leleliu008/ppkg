#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fnmatch.h>

#include "core/fs.h"
#include "ppkg.h"

int ppkg_logs(const char * packageName) {
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

    size_t  packageMetadataDirLength = userHomeDirLength + strlen(packageName) + 24;
    char    packageMetadataDir[packageMetadataDirLength];
    memset (packageMetadataDir, 0, packageMetadataDirLength);
    sprintf(packageMetadataDir, "%s/.ppkg/installed/%s/.ppkg", userHomeDir, packageName);

    size_t  receiptFilePathLength = packageMetadataDirLength + 13;
    char    receiptFilePath[receiptFilePathLength];
    memset (receiptFilePath, 0, receiptFilePathLength);
    sprintf(receiptFilePath, "%s/receipt.yml", packageMetadataDir);

    if (!exists_and_is_a_regular_file(receiptFilePath)) {
        return PPKG_PACKAGE_IS_NOT_INSTALLED;
    }

    if (!exists_and_is_a_regular_file(receiptFilePath)) {
        return PPKG_PACKAGE_IS_NOT_INSTALLED;
    }

    DIR           * dir;
    struct dirent * dir_entry;

    dir = opendir(packageMetadataDir);

    if (dir == NULL) {
        perror(packageMetadataDir);
        return PPKG_ERROR;
    }

    while ((dir_entry = readdir(dir))) {
        //puts(dir_entry->d_name);
        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t  cmdLength = packageMetadataDirLength + strlen(dir_entry->d_name) + 6;
        char    cmd[cmdLength];
        memset (cmd, 0, cmdLength);
        sprintf(cmd, "bat %s/%s", packageMetadataDir, dir_entry->d_name);


        resultCode = system(cmd);

        if (resultCode != 0) {
            break;
        }
    }

    closedir(dir);

    return resultCode;
}
