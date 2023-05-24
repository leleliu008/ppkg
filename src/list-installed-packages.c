#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

#include "core/log.h"
#include "ppkg.h"

int ppkg_list_the_installed_packages() {
    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    struct stat st;

    size_t ppkgInstalledDirLength = userHomeDirLength + 17U; 
    char   ppkgInstalledDir[ppkgInstalledDirLength];
    snprintf(ppkgInstalledDir, ppkgInstalledDirLength, "%s/.ppkg/installed", userHomeDir);

    if (stat(ppkgInstalledDir, &st) != 0 || (!S_ISDIR(st.st_mode))) {
        return PPKG_OK;
    }

    DIR * dir = opendir(ppkgInstalledDir);

    if (dir == NULL) {
        perror(ppkgInstalledDir);
        return PPKG_ERROR;
    }

    struct dirent * dir_entry;

    for (;;) {
        errno = 0;

        dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                break;
            } else {
                perror(ppkgInstalledDir);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t receiptFilePathLength = ppkgInstalledDirLength + strlen(dir_entry->d_name) + 20U;
        char   receiptFilePath[receiptFilePathLength];
        snprintf(receiptFilePath, receiptFilePathLength, "%s/%s/.ppkg/receipt.yml", ppkgInstalledDir, dir_entry->d_name);

        if (stat(receiptFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            printf("%s\n", dir_entry->d_name);
        }
    }

    return PPKG_OK;
}
