#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "core/fs.h"
#include "ppkg.h"

static int record_installed_files_r(const char * dirPath, size_t offset, FILE * installedManifestFile) {
    if (dirPath == NULL) {
        return PPKG_ARG_IS_NULL;
    }

    if (strcmp(dirPath, "") == 0) {
        return PPKG_ARG_IS_EMPTY;
    }

    DIR           * dir;
    struct dirent * dir_entry;

    dir = opendir(dirPath);

    if (dir == NULL) {
        perror(dirPath);
        return PPKG_ERROR;
    }

    int r = 0;

    while ((dir_entry = readdir(dir))) {
        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t  filePathLength = strlen(dirPath) + strlen(dir_entry->d_name) + 2;
        char    filePath[filePathLength];
        memset( filePath, 0, filePathLength);
        snprintf(filePath, filePathLength, "%s/%s", dirPath, dir_entry->d_name);

        struct stat st;

        r = stat(filePath, &st);

        if (r == 0) {
            if (S_ISDIR(st.st_mode)) {
                fprintf(installedManifestFile, "d|%s/\n", &filePath[offset]);

                r = record_installed_files_r(filePath, offset, installedManifestFile);

                if (r != 0) {
                    break;
                }
            } else {
                fprintf(installedManifestFile, "f|%s\n", &filePath[offset]);
            }
        } else {
            perror(filePath);
            break;
        }
    }

    closedir(dir);

    return r;
}

int record_installed_files(const char * installedDirPath) {
    size_t installedDirLength = strlen(installedDirPath);

    size_t  installedManifestFilePathLength = installedDirLength + 20;
    char    installedManifestFilePath[installedManifestFilePathLength];
    memset (installedManifestFilePath, 0, installedManifestFilePathLength);
    snprintf(installedManifestFilePath, installedManifestFilePathLength, "%s/.ppkg/manifest.txt", installedDirPath);

    if (exists_and_is_a_regular_file(installedManifestFilePath)) {
        return PPKG_OK;
    }

    FILE * installedManifestFile = fopen(installedManifestFilePath, "w");

    if (installedManifestFile == NULL) {
        perror(installedManifestFilePath);
        return PPKG_ERROR;
    }

    int resultCode = record_installed_files_r(installedDirPath, installedDirLength + 1, installedManifestFile);

    fclose(installedManifestFile);

    return resultCode;
}
