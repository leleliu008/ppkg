#include <string.h>
#include <sys/stat.h>
#include <libgen.h>

#include "core/regex/regex.h"
#include "core/sysinfo.h"
#include "core/untar.h"
#include "core/http.h"
#include "core/util.h"
#include "core/log.h"
#include "core/fs.h"
#include "ppkg.h"

int ppkg_upgrade_self(bool verbose) {
    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    ////////////////////////////////////////////////////////////////

    size_t  ppkgHomeDirLength = userHomeDirLength + 7;
    char    ppkgHomeDir[ppkgHomeDirLength];
    memset (ppkgHomeDir, 0, ppkgHomeDirLength);
    sprintf(ppkgHomeDir, "%s/.ppkg", userHomeDir);

    if (!exists_and_is_a_directory(ppkgHomeDir)) {
        if (mkdir(ppkgHomeDir, S_IRWXU) != 0) {
            perror(ppkgHomeDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////

    size_t  ppkgTmpDirLength = ppkgHomeDirLength + 5;
    char    ppkgTmpDir[ppkgTmpDirLength];
    memset (ppkgTmpDir, 0, ppkgTmpDirLength);
    sprintf(ppkgTmpDir, "%s/tmp", ppkgHomeDir);

    if (!exists_and_is_a_directory(ppkgTmpDir)) {
        if (mkdir(ppkgTmpDir, S_IRWXU) != 0) {
            perror(ppkgTmpDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    const char * githubApiUrl = "https://api.github.com/repos/leleliu008/ppkg/releases/latest";

    size_t  githubApiResultJsonFilePathLength = ppkgTmpDirLength + 13;
    char    githubApiResultJsonFilePath[githubApiResultJsonFilePathLength];
    memset (githubApiResultJsonFilePath, 0, githubApiResultJsonFilePathLength);
    sprintf(githubApiResultJsonFilePath, "%s/latest.json", ppkgTmpDir);

    if (http_fetch_to_file(githubApiUrl, githubApiResultJsonFilePath, verbose, verbose) != 0) {
        return PPKG_NETWORK_ERROR;
    }

    FILE * file = fopen(githubApiResultJsonFilePath, "r");

    if (file == NULL) {
        perror(githubApiResultJsonFilePath);
        return PPKG_ERROR;
    }

    char * latestVersion = NULL;

    char buf[30];

    size_t j = 0;

    while ((fgets(buf, 30, file)) != NULL) {
        if (regex_matched(buf, "^[[:space:]]*\"tag_name\"")) {
            size_t length = strlen(buf);
            for (size_t i = 10; i < length; i++) {
                if (j == 0) {
                    if (buf[i] >= '0' && buf[i] <= '9') {
                        j = i;
                    }
                } else {
                    if (buf[i] == '"') {
                        buf[i] = '\0';
                        latestVersion = &buf[j];
                        break;
                    }
                }
            }
            break;
        }
    }

    fclose(file);

    if (latestVersion == NULL) {
        fprintf(stderr, "can not get latest version.\n");
        return PPKG_ERROR;
    }

    if (strcmp(latestVersion, PPKG_VERSION) == 0) {
        LOG_SUCCESS1("this software is already the latest version.");
        return PPKG_OK;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    char * osType = NULL;

    if (sysinfo_type(&osType) != 0) {
        return PPKG_ERROR;
    }

    char * osArch = NULL;

    if (sysinfo_arch(&osArch) != 0) {
        return PPKG_ERROR;
    }

    size_t  latestVersionLength = strlen(latestVersion);

    size_t  tarballFileNameLength = latestVersionLength + strlen(osType) + strlen(osArch) + 15;
    char    tarballFileName[tarballFileNameLength];
    memset( tarballFileName, 0, tarballFileNameLength);
    sprintf(tarballFileName, "ppkg-%s-%s-%s.tar.xz", latestVersion, osType, osArch);

    size_t  tarballUrlLength = tarballFileNameLength + latestVersionLength + 55;
    char    tarballUrl[tarballUrlLength];
    memset( tarballUrl, 0, tarballUrlLength);
    sprintf(tarballUrl, "https://github.com/leleliu008/ppkg/releases/download/%s/%s", latestVersion, tarballFileName);

    size_t  tarballFilePathLength = ppkgTmpDirLength + tarballFileNameLength + 2;
    char    tarballFilePath[tarballFilePathLength];
    memset (tarballFilePath, 0, tarballFilePathLength);
    sprintf(tarballFilePath, "%s/%s", ppkgTmpDir, tarballFileName);

    free(osType);
    free(osArch);

    if (http_fetch_to_file(tarballUrl, tarballFilePath, verbose, verbose) != 0) {
        return PPKG_NETWORK_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////

    size_t  tarballExtractDirLength = tarballFilePathLength + 3;
    char    tarballExtractDir[tarballExtractDirLength];
    memset (tarballExtractDir, 0, tarballExtractDirLength);
    sprintf(tarballExtractDir, "%s.d", tarballFilePath);

    int resultCode = untar_extract(tarballExtractDir, tarballFilePath, 0, verbose, 1);

    if (resultCode != ARCHIVE_OK) {
        return resultCode;
    }

    size_t  upgradableExecutableFilePathLength = tarballExtractDirLength + 10;
    char    upgradableExecutableFilePath[upgradableExecutableFilePathLength];
    memset (upgradableExecutableFilePath, 0, upgradableExecutableFilePathLength);
    sprintf(upgradableExecutableFilePath, "%s/bin/ppkg", tarballExtractDir);

    printf("the latest version of executable was downloaded to %s\n", upgradableExecutableFilePath);

    return PPKG_OK;
}
