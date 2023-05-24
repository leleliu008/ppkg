#include <math.h>
#include <string.h>
#include <sys/stat.h>

#include "core/sysinfo.h"
#include "core/http.h"
#include "core/tar.h"
#include "core/log.h"
#include "ppkg.h"

int ppkg_upgrade_self(bool verbose) {
    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    ////////////////////////////////////////////////////////////////

    struct stat st;

    size_t ppkgHomeDirLength = userHomeDirLength + 7U;
    char   ppkgHomeDir[ppkgHomeDirLength];
    snprintf(ppkgHomeDir, ppkgHomeDirLength, "%s/.ppkg", userHomeDir);

    if (stat(ppkgHomeDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", ppkgHomeDir);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(ppkgHomeDir, S_IRWXU) != 0) {
            perror(ppkgHomeDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////

    size_t ppkgTmpDirLength = ppkgHomeDirLength + 5U;
    char   ppkgTmpDir[ppkgTmpDirLength];
    snprintf(ppkgTmpDir, ppkgTmpDirLength, "%s/tmp", ppkgHomeDir);

    if (stat(ppkgTmpDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", ppkgTmpDir);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(ppkgTmpDir, S_IRWXU) != 0) {
            perror(ppkgTmpDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    const char * githubApiUrl = "https://api.github.com/repos/leleliu008/ppkg/releases/latest";

    size_t githubApiResultJsonFilePathLength = ppkgTmpDirLength + 13U;
    char   githubApiResultJsonFilePath[githubApiResultJsonFilePathLength];
    snprintf(githubApiResultJsonFilePath, githubApiResultJsonFilePathLength, "%s/latest.json", ppkgTmpDir);

    int ret = http_fetch_to_file(githubApiUrl, githubApiResultJsonFilePath, verbose, verbose);

    if (ret != PPKG_OK) {
        return ret;
    }

    FILE * file = fopen(githubApiResultJsonFilePath, "r");

    if (file == NULL) {
        perror(githubApiResultJsonFilePath);
        return PPKG_ERROR;
    }

    char * latestVersion = NULL;

    char * p = NULL;

    char line[30];

    for (;;) {
        p = fgets(line, 30, file);

        if (p == NULL) {
            if (ferror(file)) {
                perror(githubApiResultJsonFilePath);
                goto finalize;
            } else {
                break;
            }
        }

        for (;;) {
            if (p[0] <= 32) { // non-printable ASCII characters and space
                p++;
            } else {
                break;
            }
        }

        if (strncmp(p, "\"tag_name\"", 10) == 0) {
            p += 10;

            for (;;) {
                if (p[0] == '\0') {
                    fprintf(stderr, "%s return invalid json.\n", githubApiUrl);
                    return PPKG_ERROR;
                }

                if (p[0] == '"') { // found left double quote
                    p++;
                    break;
                } else {
                    p++;
                }
            }

            size_t n = 0;
            char * q = p;

            for (;;) {
                if (q[n] == '\0') {
                    fprintf(stderr, "%s return invalid json.\n", githubApiUrl);
                    return PPKG_ERROR;
                }

                if (q[n] == '"') { // found right double quote
                    q[n] = '\0';
                    latestVersion = &q[0];
                    goto finalize;
                } else {
                    n++;
                }
            }
        }
    }

finalize:
    fclose(file);

    printf("latestVersion=%s\n", latestVersion);

    if (latestVersion == NULL) {
        fprintf(stderr, "%s return json has no tag_name key.\n", githubApiUrl);
        return PPKG_ERROR;
    }

    if (strcmp(latestVersion, PPKG_VERSION) == 0) {
        LOG_SUCCESS1("this software is already the latest version.");
        return PPKG_OK;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    char osType[31] = {0};

    ret = sysinfo_type(osType, 30);

    if (ret != PPKG_OK) {
        return ret;
    }

    char osArch[31] = {0};

    ret = sysinfo_arch(osArch, 30);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t latestVersionLength = strlen(latestVersion);

    size_t tarballFileNameLength = latestVersionLength + strlen(osType) + strlen(osArch) + 15U;
    char   tarballFileName[tarballFileNameLength];
    snprintf(tarballFileName, tarballFileNameLength, "ppkg-%s-%s-%s.tar.xz", latestVersion, osType, osArch);

    size_t tarballUrlLength = tarballFileNameLength + latestVersionLength + 55U;
    char   tarballUrl[tarballUrlLength];
    snprintf(tarballUrl, tarballUrlLength, "https://github.com/leleliu008/ppkg/releases/download/%s/%s", latestVersion, tarballFileName);

    size_t tarballFilePathLength = ppkgTmpDirLength + tarballFileNameLength + 2U;
    char   tarballFilePath[tarballFilePathLength];
    snprintf(tarballFilePath, tarballFilePathLength, "%s/%s", ppkgTmpDir, tarballFileName);

    ret = http_fetch_to_file(tarballUrl, tarballFilePath, verbose, verbose);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////////

    size_t tarballExtractDirLength = tarballFilePathLength + 3U;
    char   tarballExtractDir[tarballExtractDirLength];
    snprintf(tarballExtractDir, tarballExtractDirLength, "%s.d", tarballFilePath);

    ret = tar_extract(tarballExtractDir, tarballFilePath, 0, verbose, 1);

    if (ret != 0) {
        return abs(ret) + PPKG_ERROR_ARCHIVE_BASE;
    }

    size_t upgradableExecutableFilePathLength = tarballExtractDirLength + 10U;
    char   upgradableExecutableFilePath[upgradableExecutableFilePathLength];
    snprintf(upgradableExecutableFilePath, upgradableExecutableFilePathLength, "%s/bin/ppkg", tarballExtractDir);

    printf("the latest version of executable was downloaded to %s\n", upgradableExecutableFilePath);

    return PPKG_OK;
}
