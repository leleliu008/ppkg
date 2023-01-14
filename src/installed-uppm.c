#include <string.h>
#include <sys/stat.h>
#include <libgen.h>

#include "core/regex/regex.h"
#include "core/sysinfo.h"
#include "core/tar.h"
#include "core/http.h"
#include "core/util.h"
#include "core/log.h"
#include "core/fs.h"
#include "ppkg.h"

int ppkg_install_uppm(bool verbose) {
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
    snprintf(ppkgHomeDir, ppkgHomeDirLength, "%s/.ppkg", userHomeDir);

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
    snprintf(ppkgTmpDir, ppkgTmpDirLength, "%s/tmp", ppkgHomeDir);

    if (!exists_and_is_a_directory(ppkgTmpDir)) {
        if (mkdir(ppkgTmpDir, S_IRWXU) != 0) {
            perror(ppkgTmpDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    const char * githubApiUrl = "https://api.github.com/repos/leleliu008/uppm/releases/latest";

    size_t  githubApiResultJsonFilePathLength = ppkgTmpDirLength + 18;
    char    githubApiResultJsonFilePath[githubApiResultJsonFilePathLength];
    memset (githubApiResultJsonFilePath, 0, githubApiResultJsonFilePathLength);
    snprintf(githubApiResultJsonFilePath, githubApiResultJsonFilePathLength, "%s/latest-uppm.json", ppkgTmpDir);

    char * latestVersion = NULL;

    char buf[30];

    if (http_fetch_to_file(githubApiUrl, githubApiResultJsonFilePath, verbose, verbose) == 0) {
        FILE * file = fopen(githubApiResultJsonFilePath, "r");

        if (file == NULL) {
            perror(githubApiResultJsonFilePath);
            return PPKG_ERROR;
        }

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
    }

    if (latestVersion == NULL) {
        latestVersion = (char*)"1.0.0";
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    char osType[31] = {0};

    if (sysinfo_type(osType, 30) != 0) {
        return PPKG_ERROR;
    }

    char osArch[31] = {0};

    if (sysinfo_arch(osArch, 30) != 0) {
        return PPKG_ERROR;
    }

    size_t  latestVersionLength = strlen(latestVersion);

    size_t  tarballFileNameLength = latestVersionLength + strlen(osType) + strlen(osArch) + 15;
    char    tarballFileName[tarballFileNameLength];
    memset( tarballFileName, 0, tarballFileNameLength);
    snprintf(tarballFileName, tarballFileNameLength, "uppm-%s-%s-%s.tar.xz", latestVersion, osType, osArch);

    size_t  tarballUrlLength = tarballFileNameLength + latestVersionLength + 55;
    char    tarballUrl[tarballUrlLength];
    memset( tarballUrl, 0, tarballUrlLength);
    snprintf(tarballUrl, tarballUrlLength, "https://github.com/leleliu008/uppm/releases/download/%s/%s", latestVersion, tarballFileName);

    size_t  tarballFilePathLength = ppkgTmpDirLength + tarballFileNameLength + 2;
    char    tarballFilePath[tarballFilePathLength];
    memset (tarballFilePath, 0, tarballFilePathLength);
    snprintf(tarballFilePath, tarballFilePathLength, "%s/%s", ppkgTmpDir, tarballFileName);

    if (http_fetch_to_file(tarballUrl, tarballFilePath, verbose, verbose) != 0) {
        return PPKG_NETWORK_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////

    size_t  ppkgCoreDirLength = ppkgHomeDirLength + 6;
    char    ppkgCoreDir[ppkgCoreDirLength];
    memset (ppkgCoreDir, 0, ppkgCoreDirLength);
    snprintf(ppkgCoreDir, ppkgCoreDirLength, "%s/core", ppkgHomeDir);

    int resultCode = tar_extract(ppkgCoreDir, tarballFilePath, 0, verbose, 1);

    if (resultCode != ARCHIVE_OK) {
        return resultCode;
    }

    size_t  cacertPemFilePathLength = ppkgTmpDirLength + 12;
    char    cacertPemFilePath[cacertPemFilePathLength];
    memset (cacertPemFilePath, 0, cacertPemFilePathLength);
    snprintf(cacertPemFilePath, cacertPemFilePathLength, "%s/cacert.pem", ppkgTmpDir);

    if (http_fetch_to_file("https://curl.se/ca/cacert.pem", cacertPemFilePath, verbose, verbose) != 0) {
        return PPKG_NETWORK_ERROR;
    }

    size_t  cacertPemFilePath2Length = ppkgTmpDirLength + 12;
    char    cacertPemFilePath2[cacertPemFilePath2Length];
    memset (cacertPemFilePath2, 0, cacertPemFilePath2Length);
    snprintf(cacertPemFilePath2, cacertPemFilePath2Length, "%s/cacert.pem", ppkgHomeDir);

    if (cp(cacertPemFilePath, cacertPemFilePath2) != 0) {
        return PPKG_ERROR;
    }

    setenv("SSL_CERT_FILE", cacertPemFilePath2, 1);

    size_t  cmdLength = ppkgCoreDirLength + 17;
    char    cmd[cmdLength];
    memset (cmd, 0, cmdLength);
    snprintf(cmd, cmdLength, "%s/bin/uppm update", ppkgCoreDir);

    if (system(cmd) != 0) {
        return PPKG_ERROR;
    }

    return PPKG_OK;
}
