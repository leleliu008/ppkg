#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>

#include "core/log.h"

#include "ppkg.h"

int ppkg_list_the_installed_packages() {
    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    int ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    struct stat st;

    size_t   ppkgInstalledDIRLength = ppkgHomeDIRLength + 11U; 
    char     ppkgInstalledDIR[ppkgInstalledDIRLength];
    snprintf(ppkgInstalledDIR, ppkgInstalledDIRLength, "%s/installed", ppkgHomeDIR);

    if (stat(ppkgInstalledDIR, &st) != 0 || (!S_ISDIR(st.st_mode))) {
        return PPKG_OK;
    }

    DIR * dir = opendir(ppkgInstalledDIR);

    if (dir == NULL) {
        perror(ppkgInstalledDIR);
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
                perror(ppkgInstalledDIR);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t   packageInstalledDIRLength = ppkgInstalledDIRLength + strlen(dir_entry->d_name) + 2U;
        char     packageInstalledDIR[packageInstalledDIRLength];
        snprintf(packageInstalledDIR, packageInstalledDIRLength, "%s/%s", ppkgInstalledDIR, dir_entry->d_name);

        if (lstat(packageInstalledDIR, &st) == 0) {
            if (!S_ISLNK(st.st_mode)) {
                continue;
            }
        } else {
            continue;
        }

        size_t   receiptFilePathLength = packageInstalledDIRLength + 20U;
        char     receiptFilePath[receiptFilePathLength];
        snprintf(receiptFilePath, receiptFilePathLength, "%s/.ppkg/RECEIPT.yml", packageInstalledDIR);

        if (lstat(receiptFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            printf("%s\n", dir_entry->d_name);
        }
    }

    return PPKG_OK;
}
