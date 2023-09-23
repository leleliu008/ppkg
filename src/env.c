#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>

#include "core/self.h"

#include "ppkg.h"

static int ppkg_list_dirs(const char * installedDIR, size_t installedDIRLength, const char * sub) {
    DIR * dir = opendir(installedDIR);

    if (dir == NULL) {
        perror(installedDIR);
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
                perror(installedDIR);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        //puts(dir_entry->d_name);

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t packageInstalledDIRLength = installedDIRLength + strlen(dir_entry->d_name) + 2U;
        char   packageInstalledDIR[packageInstalledDIRLength];
        snprintf(packageInstalledDIR, packageInstalledDIRLength, "%s/%s", installedDIR, dir_entry->d_name);

        size_t receiptFilePathLength = packageInstalledDIRLength + 20U;
        char   receiptFilePath[receiptFilePathLength];
        snprintf(receiptFilePath, receiptFilePathLength, "%s/.ppkg/RECEIPT.yml", packageInstalledDIR);

        if (stat(receiptFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            if ((sub == NULL) || (sub[0] == '\0')) {
                printf("%s\n", packageInstalledDIR);
            } else {
                size_t subDIRLength = packageInstalledDIRLength + strlen(sub) + 2U;
                char   subDIR[subDIRLength];
                snprintf(subDIR, subDIRLength, "%s/%s", packageInstalledDIR, sub);

                if (stat(subDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
                    printf("%s\n", subDIR);
                }
            }
        }
    }

    return PPKG_OK;
}

int ppkg_env(bool verbose) {
    char   ppkgHomeDIR[256] = {0};
    size_t ppkgHomeDIRLength;

    int ret = ppkg_home_dir(ppkgHomeDIR, 255, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    printf("ppkg.version : %s\n", PPKG_VERSION);
    printf("ppkg.homedir : %s\n", ppkgHomeDIR);

    char * selfRealPath = self_realpath();

    if (selfRealPath == NULL) {
        perror(NULL);
        return PPKG_ERROR;
    }

    printf("ppkg.exepath : %s\n", selfRealPath);

    free(selfRealPath);

    printf("ppkg.website : %s\n", "https://github.com/leleliu008/ppkg");

    if (!verbose) {
        return PPKG_OK;
    }

    struct stat st;

    size_t   installedDIRLength = ppkgHomeDIRLength + 11U;
    char     installedDIR[installedDIRLength];
    snprintf(installedDIR, installedDIRLength, "%s/installed", ppkgHomeDIR);

    if (stat(installedDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "%s was expected to be a directory, but it was not.\n", installedDIR);
            return PPKG_ERROR;
        }
    } else {
        return PPKG_OK;
    }

    printf("\nbinDirs:\n");
    ppkg_list_dirs(installedDIR, installedDIRLength, "bin");

    printf("\nlibDirs:\n");
    ppkg_list_dirs(installedDIR, installedDIRLength, "lib");

    printf("\naclocalDirs:\n");
    ppkg_list_dirs(installedDIR, installedDIRLength, "share/aclocal");
    
    return PPKG_OK;
}
