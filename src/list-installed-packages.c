#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fnmatch.h>

#include "core/log.h"
#include "core/fs.h"
#include "ppkg.h"

int ppkg_list_the_installed_packages() {
    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL || strcmp(userHomeDir, "") == 0) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    size_t  installedDirLength = userHomeDirLength + 17; 
    char    installedDir[installedDirLength];
    memset (installedDir, 0, installedDirLength);
    sprintf(installedDir, "%s/.ppkg/installed", userHomeDir);

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
        sprintf(receiptFilePath, "%s/%s/.ppkg/receipt.yml", installedDir, dir_entry->d_name);

        if (exists_and_is_a_regular_file(receiptFilePath)) {
            printf("%s\n", dir_entry->d_name);
        }
    }

    closedir(dir);

    return PPKG_OK;
}
