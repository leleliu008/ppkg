#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

#include "core/self.h"

#include "ppkg.h"

static int ppkg_list_dirs(const char * installedDir, size_t installedDirLength, const char * sub) {
    DIR * dir = opendir(installedDir);

    if (dir == NULL) {
        perror(installedDir);
        return PPKG_ERROR;
    }

    struct stat st;

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                break;
            } else {
                perror(installedDir);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        //puts(dir_entry->d_name);

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t packageInstalledDirLength = installedDirLength + strlen(dir_entry->d_name) + 2U;
        char   packageInstalledDir[packageInstalledDirLength];
        snprintf(packageInstalledDir, packageInstalledDirLength, "%s/%s", installedDir, dir_entry->d_name);

        size_t receiptFilePathLength = packageInstalledDirLength + 20U;
        char   receiptFilePath[receiptFilePathLength];
        snprintf(receiptFilePath, receiptFilePathLength, "%s/.ppkg/receipt.yml", packageInstalledDir);

        if (stat(receiptFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            if ((sub == NULL) || (sub[0] == '\0')) {
                printf("%s\n", packageInstalledDir);
            } else {
                size_t subDirLength = packageInstalledDirLength + strlen(sub) + 2U;
                char   subDir[subDirLength];
                snprintf(subDir, subDirLength, "%s/%s", packageInstalledDir, sub);

                if (stat(subDir, &st) == 0 && S_ISDIR(st.st_mode)) {
                    printf("%s\n", subDir);
                }
            }
        }
    }

    return PPKG_OK;
}

int ppkg_env(bool verbose) {
    char   ppkgHomeDir[256];
    size_t ppkgHomeDirLength;

    int ret = ppkg_home_dir(ppkgHomeDir, 256, &ppkgHomeDirLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    printf("ppkg.vers : %s\n", PPKG_VERSION);
    printf("ppkg.home : %s\n", ppkgHomeDir);

    char * selfRealPath = self_realpath();

    if (selfRealPath == NULL) {
        perror(NULL);
        return PPKG_ERROR;
    }

    printf("ppkg.path : %s\n", selfRealPath);

    free(selfRealPath);

    printf("ppkg.link : %s\n", "https://github.com/leleliu008/ppkg");

    if (!verbose) {
        return PPKG_OK;
    }

    struct stat st;

    size_t   installedDirLength = ppkgHomeDirLength + 11U;
    char     installedDir[installedDirLength];
    snprintf(installedDir, installedDirLength, "%s/installed", ppkgHomeDir);

    if (stat(installedDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", installedDir);
            return PPKG_ERROR;
        }
    } else {
        return PPKG_OK;
    }

    printf("\nbinDirs:\n");
    ppkg_list_dirs(installedDir, installedDirLength, "bin");

    printf("\nlibDirs:\n");
    ppkg_list_dirs(installedDir, installedDirLength, "lib");

    printf("\naclocalDirs:\n");
    ppkg_list_dirs(installedDir, installedDirLength, "share/aclocal");
    
    return PPKG_OK;
}
