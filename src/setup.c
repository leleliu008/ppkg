#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "core/regex/regex.h"
#include "core/mkdir_p.h"
#include "core/sysinfo.h"
#include "core/http.h"
#include "core/tar.h"
#include "core/log.h"
#include "core/cp.h"

#include "ppkg.h"

int ppkg_setup(bool verbose) {
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

    size_t   ppkgHomeDirLength = userHomeDirLength + 7U;
    char     ppkgHomeDir[ppkgHomeDirLength];
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

    size_t   ppkgTmpDirLength = ppkgHomeDirLength + 5U;
    char     ppkgTmpDir[ppkgTmpDirLength];
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

    const char * const githubApiUrl = "https://api.github.com/repos/leleliu008/uppm/releases/latest";

    size_t   githubApiResultJsonFilePathLength = ppkgTmpDirLength + 18U;
    char     githubApiResultJsonFilePath[githubApiResultJsonFilePathLength];
    snprintf(githubApiResultJsonFilePath, githubApiResultJsonFilePathLength, "%s/latest-uppm.json", ppkgTmpDir);

    char * latestReleaseName = NULL;

    char buf[30];

    if (http_fetch_to_file(githubApiUrl, githubApiResultJsonFilePath, verbose, verbose) == 0) {
        FILE * file = fopen(githubApiResultJsonFilePath, "r");

        if (file == NULL) {
            perror(githubApiResultJsonFilePath);
            return PPKG_ERROR;
        }

        size_t j = 0;

        for (;;) {
            if (fgets(buf, 30, file) == NULL) {
                if (ferror(file)) {
                    perror(githubApiResultJsonFilePath);
                    fclose(file);
                    return PPKG_ERROR;
                } else {
                    break;
                }
            }

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
                            latestReleaseName = &buf[j];
                            break;
                        }
                    }
                }
                break;
            }
        }

        fclose(file);
    }

    if (latestReleaseName == NULL) {
        latestReleaseName = (char*)"0.10.4+2f295170a1d24bc4736af99f4509c4b2a1c264fb";
    }

    char latestReleaseVersion[10] = {0};

    for (int i = 0; i < 10; i++) {
        char c = latestReleaseName[i];

        if (c == '+') {
            break;
        }

        latestReleaseVersion[i] = latestReleaseName[i];
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

    size_t  latestReleaseVersionLength = strlen(latestReleaseVersion);

    size_t   tarballFileNameLength = latestReleaseVersionLength + strlen(osType) + strlen(osArch) + 15U;
    char     tarballFileName[tarballFileNameLength];
    snprintf(tarballFileName, tarballFileNameLength, "uppm-%s-%s-%s.tar.xz", latestReleaseVersion, osType, osArch);

    size_t   tarballUrlLength = tarballFileNameLength + strlen(latestReleaseName) + 55U;
    char     tarballUrl[tarballUrlLength];
    snprintf(tarballUrl, tarballUrlLength, "https://github.com/leleliu008/uppm/releases/download/%s/%s", latestReleaseName, tarballFileName);

    size_t   tarballFilePathLength = ppkgTmpDirLength + tarballFileNameLength + 2U;
    char     tarballFilePath[tarballFilePathLength];
    snprintf(tarballFilePath, tarballFilePathLength, "%s/%s", ppkgTmpDir, tarballFileName);

    int ret = http_fetch_to_file(tarballUrl, tarballFilePath, verbose, verbose);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////////

    size_t   ppkgCoreDirLength = ppkgHomeDirLength + 6U;
    char     ppkgCoreDir[ppkgCoreDirLength];
    snprintf(ppkgCoreDir, ppkgCoreDirLength, "%s/core", ppkgHomeDir);

    ret = tar_extract(ppkgCoreDir, tarballFilePath, 0, verbose, 1);

    if (ret != ARCHIVE_OK) {
        return ret;
    }

    size_t   cacertPemFilePathLength = ppkgTmpDirLength + 12U;
    char     cacertPemFilePath[cacertPemFilePathLength];
    snprintf(cacertPemFilePath, cacertPemFilePathLength, "%s/cacert.pem", ppkgTmpDir);

    ret = http_fetch_to_file("https://curl.se/ca/cacert.pem", cacertPemFilePath, verbose, verbose);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t   cacertDIRLength = ppkgCoreDirLength + 15U;
    char     cacertDIR[cacertDIRLength];
    snprintf(cacertDIR, cacertDIRLength, "%s/etc/ssl/certs", ppkgCoreDir);

    if (mkdir_p(cacertDIR, verbose) != 0) {
        return PPKG_ERROR;
    }

    size_t   cacertPemFilePath2Length = cacertDIRLength + 12U;
    char     cacertPemFilePath2[cacertPemFilePath2Length];
    snprintf(cacertPemFilePath2, cacertPemFilePath2Length, "%s/cacert.pem", cacertDIR);

    if (copy_file(cacertPemFilePath, cacertPemFilePath2) != 0) {
        return PPKG_ERROR;
    }

    if (setenv("SSL_CERT_FILE", cacertPemFilePath2, 1) != 0) {
        perror("SSL_CERT_FILE");
        return PPKG_ERROR;
    }

    size_t   uppmCmdPathLength = ppkgCoreDirLength + 10U;
    char     uppmCmdPath[uppmCmdPathLength];
    snprintf(uppmCmdPath, uppmCmdPathLength, "%s/bin/uppm", ppkgCoreDir);

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (pid == 0) {
        char* argv[3] = { uppmCmdPath, (char*)"update", NULL };
        execvp(argv[0], argv);
        perror(argv[0]);
        exit(127);
    } else {
        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (childProcessExitStatusCode == 0) {
            return PPKG_OK;
        } else {
            size_t   cmdLength = uppmCmdPathLength + 6U;
            char     cmd[cmdLength];
            snprintf(cmd, cmdLength, "%s update", uppmCmdPath);

            if (WIFEXITED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command '%s' exit with status code: %d\n", cmd, WEXITSTATUS(childProcessExitStatusCode));
            } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command '%s' killed by signal: %d\n", cmd, WTERMSIG(childProcessExitStatusCode));
            } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command '%s' stopped by signal: %d\n", cmd, WSTOPSIG(childProcessExitStatusCode));
            }

            return PPKG_ERROR;
        }
    }
}
