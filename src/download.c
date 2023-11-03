#include <time.h>
#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <limits.h>
#include <sys/stat.h>

#include "core/tar.h"

#include "sha256sum.h"

#include "ppkg.h"

int ppkg_download(const char * url, const char * expectedSHA256SUM, const char * downloadDIR, const char * unpackDIR, size_t stripComponentsNumber, bool verbose) {
    if (url == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (url[0] == '\0') {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (expectedSHA256SUM == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (expectedSHA256SUM[0] == '\0') {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (strlen(expectedSHA256SUM) != 64U) {
        return PPKG_ERROR_ARG_IS_INVALID;
    }

    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    int ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////

    size_t defaultDownloadDIRLength = ppkgHomeDIRLength + 11U;
    char   defaultDownloadDIR[defaultDownloadDIRLength];

    ret = snprintf(defaultDownloadDIR, defaultDownloadDIRLength, "%s/downloads", ppkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (downloadDIR == NULL) {
        downloadDIR = defaultDownloadDIR;
    }

    size_t downloadDIRLength = strlen(downloadDIR);

    struct stat st;

    if (lstat(downloadDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(downloadDIR) != 0) {
                perror(downloadDIR);
                return PPKG_ERROR;
            }

            if (mkdir(downloadDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(downloadDIR);
                    return PPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(downloadDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(downloadDIR);
                return PPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////

    char fileNameExtension[21] = {0};

    ret = ppkg_examine_file_extension_from_url(url, fileNameExtension, 20);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////

    size_t fileNameLength = strlen(fileNameExtension) + 65U;
    char   fileName[fileNameLength];

    ret = snprintf(fileName, fileNameLength, "%s%s", expectedSHA256SUM, fileNameExtension);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    size_t filePathLength = downloadDIRLength + fileNameLength + 1U;
    char   filePath[filePathLength];

    ret = snprintf(filePath, filePathLength, "%s/%s", downloadDIR, fileName);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////

    bool needFetch = true;

    if (lstat(filePath, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            char actualSHA256SUM[65] = {0};

            if (sha256sum_of_file(actualSHA256SUM, filePath) != 0) {
                return PPKG_ERROR;
            }

            if (strcmp(actualSHA256SUM, expectedSHA256SUM) == 0) {
                needFetch = false;
            }
        }
    }

    if (needFetch) {
        size_t tmpStrLength = strlen(url) + 30U;
        char   tmpStr[tmpStrLength];

        ret = snprintf(tmpStr, tmpStrLength, "%s|%ld|%d", url, time(NULL), getpid());

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        char tmpFileName[65] = {0};

        ret = sha256sum_of_string(tmpFileName, tmpStr);

        if (ret != 0) {
            return PPKG_ERROR;
        }

        size_t tmpFilePathLength = downloadDIRLength + 65U;
        char   tmpFilePath[tmpFilePathLength];

        ret = snprintf(tmpFilePath, tmpFilePathLength, "%s/%s", downloadDIR, tmpFileName);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        ret = ppkg_http_fetch_to_file(url, tmpFilePath, verbose, verbose);

        if (ret != PPKG_OK) {
            return ret;
        }

        char actualSHA256SUM[65] = {0};

        if (sha256sum_of_file(actualSHA256SUM, tmpFilePath) != 0) {
            return PPKG_ERROR;
        }

        if (strcmp(actualSHA256SUM, expectedSHA256SUM) == 0) {
            if (rename(tmpFilePath, filePath) == 0) {
                printf("%s\n", filePath);
            } else {
                if (errno == EXDEV) {
                    ret = ppkg_copy_file(tmpFilePath, filePath);

                    if (ret != PPKG_OK) {
                        return ret;
                    }
                } else {
                    perror(filePath);
                    return PPKG_ERROR;
                }
            }
        } else {
            fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", expectedSHA256SUM, actualSHA256SUM);
            return PPKG_ERROR_SHA256_MISMATCH;
        }
    } else {
        fprintf(stderr, "%s already have been fetched.\n", filePath);
    }

    //////////////////////////////////////////////////////////////////////////

    if (unpackDIR != NULL) {
        ret = tar_extract(unpackDIR, filePath, ARCHIVE_EXTRACT_TIME, verbose, stripComponentsNumber);

        if (ret != 0) {
            return abs(ret) + PPKG_ERROR_ARCHIVE_BASE;
        }
    }

    return PPKG_OK;
}
