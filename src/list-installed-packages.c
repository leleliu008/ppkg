#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>

#include "core/log.h"

#include "ppkg.h"

static int _list_dir(const PPKGTargetPlatform * targetPlatform, const char * packageInstalledRootDIR, const size_t packageInstalledRootDIRCapacity, const char * platformDIRName, const size_t platformDIRNameCapacity, const bool verbose) {
    size_t packageInstalledRootSubDIRCapacity = packageInstalledRootDIRCapacity + platformDIRNameCapacity;
    char   packageInstalledRootSubDIR[packageInstalledRootSubDIRCapacity];

    int ret = snprintf(packageInstalledRootSubDIR, packageInstalledRootSubDIRCapacity, "%s/%s", packageInstalledRootDIR, platformDIRName);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    struct stat st;

    if (!((stat(packageInstalledRootSubDIR, &st) == 0) && S_ISDIR(st.st_mode))) {
        return PPKG_OK;
    }

    DIR * dir = opendir(packageInstalledRootSubDIR);

    if (dir == NULL) {
        perror(packageInstalledRootSubDIR);
        return PPKG_ERROR;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return PPKG_OK;
            } else {
                perror(packageInstalledRootSubDIR);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t packageInstalledDIRCapacity = packageInstalledRootSubDIRCapacity + strlen(dir_entry->d_name) + 2U;
        char   packageInstalledDIR[packageInstalledDIRCapacity];

        ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", packageInstalledRootSubDIR, dir_entry->d_name);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            return PPKG_ERROR;
        }

        if (lstat(packageInstalledDIR, &st) == 0) {
            if (!S_ISLNK(st.st_mode)) {
                continue;
            }
        } else {
            continue;
        }

        size_t receiptFilePathCapacity = packageInstalledDIRCapacity + 20U;
        char   receiptFilePath[receiptFilePathCapacity];

        ret = snprintf(receiptFilePath, receiptFilePathCapacity, "%s/.ppkg/RECEIPT.yml", packageInstalledDIR);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            return PPKG_ERROR;
        }

        if (lstat(receiptFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            if (verbose) {
                ret = ppkg_installed_info(dir_entry->d_name, targetPlatform, NULL);

                if (ret != PPKG_OK) {
                    closedir(dir);
                    return ret;
                }
            } else {
                printf("%s/%s\n", platformDIRName, dir_entry->d_name);
            }
        }
    }
}

static int ppkg_list_all_installed_packages(const char * packageInstalledRootDIR, const size_t packageInstalledRootDIRCapacity, const bool verbose) {
    DIR * dir = opendir(packageInstalledRootDIR);

    if (dir == NULL) {
        perror(packageInstalledRootDIR);
        return PPKG_ERROR;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return PPKG_OK;
            } else {
                perror(packageInstalledRootDIR);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t targetPlarformSpecLength = strlen(dir_entry->d_name);
        size_t targetPlarformSpecCapacity = targetPlarformSpecLength + 1U;

        char   targetPlarformSpec[targetPlarformSpecCapacity];

        strncpy(targetPlarformSpec, dir_entry->d_name, targetPlarformSpecLength);

        targetPlarformSpec[targetPlarformSpecLength] = '\0';

        char * targetPlarformName = strtok(targetPlarformSpec, "-");
        char * targetPlarformVers = strtok(NULL, "-");
        char * targetPlarformArch = strtok(NULL, "-");
        char * p                  = strtok(NULL, "-");

        if (targetPlarformVers == NULL || targetPlarformVers[0] == '\0') {
            fprintf(stderr, "invalid target : %s\n", dir_entry->d_name);
            continue;
        }

        if (targetPlarformArch == NULL || targetPlarformArch[0] == '\0') {
            fprintf(stderr, "invalid target : %s\n", dir_entry->d_name);
            continue;
        }

        if (p != NULL) {
            fprintf(stderr, "invalid target : %s\n", dir_entry->d_name);
            continue;
        }

        PPKGTargetPlatform targetPlarform = { .name = targetPlarformName, .version = targetPlarformVers, .arch = targetPlarformArch };

        int ret = _list_dir(&targetPlarform, packageInstalledRootDIR, packageInstalledRootDIRCapacity, dir_entry->d_name, targetPlarformSpecCapacity, verbose);

        if (ret != PPKG_OK) {
            closedir(dir);
            return ret;
        }
    }
}

int ppkg_list_the_installed_packages(const PPKGTargetPlatform * targetPlatform, const bool verbose) {
    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    int ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t packageInstalledRootDIRCapacity = ppkgHomeDIRLength + 11U;
    char   packageInstalledRootDIR[packageInstalledRootDIRCapacity];

    ret = snprintf(packageInstalledRootDIR, packageInstalledRootDIRCapacity, "%s/installed", ppkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    struct stat st;

    if (!((stat(packageInstalledRootDIR, &st) == 0) && S_ISDIR(st.st_mode))) {
        return PPKG_OK;
    }

    if (targetPlatform == NULL) {
        return ppkg_list_all_installed_packages(packageInstalledRootDIR, packageInstalledRootDIRCapacity, verbose);
    }

    size_t platformDIRNameCapacity = strlen(targetPlatform->name) + strlen(targetPlatform->version) + strlen(targetPlatform->arch) + 3U;
    char   platformDIRName[platformDIRNameCapacity];

    ret = snprintf(platformDIRName, platformDIRNameCapacity, "%s-%s-%s", targetPlatform->name, targetPlatform->version, targetPlatform->arch);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    return _list_dir(targetPlatform, packageInstalledRootDIR, packageInstalledRootDIRCapacity, platformDIRName, platformDIRNameCapacity, verbose);
}
