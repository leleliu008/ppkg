#include <math.h>
#include <errno.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/wait.h>

#include "core/regex/regex.h"
#include "core/sysinfo.h"
#include "core/http.h"
#include "core/tar.h"
#include "core/log.h"

#include "ppkg.h"

int ppkg_setup(bool verbose) {
    char   ppkgHomeDIR[256] = {0};
    size_t ppkgHomeDIRLength;

    int ret = ppkg_home_dir(ppkgHomeDIR, 255, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    struct stat st;

    size_t   ppkgRunDIRLength = ppkgHomeDIRLength + 5U;
    char     ppkgRunDIR[ppkgRunDIRLength];
    snprintf(ppkgRunDIR, ppkgRunDIRLength, "%s/run", ppkgHomeDIR);

    if (stat(ppkgRunDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(ppkgRunDIR) == 0) {
                perror(ppkgRunDIR);
                return PPKG_ERROR;
            }

            if (mkdir(ppkgRunDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(ppkgRunDIR);
                    return PPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(ppkgRunDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(ppkgRunDIR);
                return PPKG_ERROR;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    size_t   sessionDIRLength = ppkgRunDIRLength + 20U;
    char     sessionDIR[sessionDIRLength];
    snprintf(sessionDIR, sessionDIRLength, "%s/%d", ppkgRunDIR, getpid());

    if (lstat(sessionDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            ret = ppkg_rm_r(sessionDIR, verbose);

            if (ret != PPKG_OK) {
                return ret;
            }

            if (mkdir(sessionDIR, S_IRWXU) != 0) {
                perror(sessionDIR);
                return PPKG_ERROR;
            }
        } else {
            if (unlink(sessionDIR) != 0) {
                perror(sessionDIR);
                return PPKG_ERROR;
            }

            if (mkdir(sessionDIR, S_IRWXU) != 0) {
                perror(sessionDIR);
                return PPKG_ERROR;
            }
        }
    } else {
        if (mkdir(sessionDIR, S_IRWXU) != 0) {
            perror(sessionDIR);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    size_t   libexecDIRLength = sessionDIRLength + 9U;
    char     libexecDIR[libexecDIRLength];
    snprintf(libexecDIR, libexecDIRLength, "%s/libexec", sessionDIR);

    if (mkdir(libexecDIR, S_IRWXU) != 0) {
        perror(libexecDIR);
        return PPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    const char * const ppkgInstallScriptUrl = "https://raw.githubusercontent.com/leleliu008/ppkg/dev/ppkg-install";

    size_t   ppkgInstallScriptFilePathLength = libexecDIRLength + 14U;
    char     ppkgInstallScriptFilePath[ppkgInstallScriptFilePathLength];
    snprintf(ppkgInstallScriptFilePath, ppkgInstallScriptFilePathLength, "%s/ppkg-install", libexecDIR);

    ret = ppkg_http_fetch_to_file(ppkgInstallScriptUrl, ppkgInstallScriptFilePath, verbose, verbose);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    const char* compilers[3] = { "cc", "c++", "objc" };

    for (int i = 0; i < 3; i++) {
        const char * compiler = compilers[i];

        size_t   wrapperCompilerSourceUrlLength = sessionDIRLength + 75U;
        char     wrapperCompilerSourceUrl[wrapperCompilerSourceUrlLength];
        snprintf(wrapperCompilerSourceUrl, wrapperCompilerSourceUrlLength, "https://raw.githubusercontent.com/leleliu008/ppkg/dev/ppkg-wrapper-%s.c", compiler);

        size_t   wrapperCompilerSourceFilePathLength = sessionDIRLength + 16U;
        char     wrapperCompilerSourceFilePath[wrapperCompilerSourceFilePathLength];
        snprintf(wrapperCompilerSourceFilePath, wrapperCompilerSourceFilePathLength, "%s/wrapper-%s.c", sessionDIR, compiler);

        ret = ppkg_http_fetch_to_file(wrapperCompilerSourceUrl, wrapperCompilerSourceFilePath, verbose, verbose);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    const char * const githubApiUrl = "https://api.github.com/repos/leleliu008/uppm/releases/latest";

    size_t   githubApiResultJsonFilePathLength = sessionDIRLength + 18U;
    char     githubApiResultJsonFilePath[githubApiResultJsonFilePathLength];
    snprintf(githubApiResultJsonFilePath, githubApiResultJsonFilePathLength, "%s/latest-uppm.json", sessionDIR);

    char * latestReleaseName = NULL;

    char buf[30];

    ret = ppkg_http_fetch_to_file(githubApiUrl, githubApiResultJsonFilePath, verbose, verbose);

    if (ret == PPKG_OK) {
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

            if (regex_matched(buf, "^[[:space:]]*\"tag_name\"") == 0) {
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
            } else {
                if (errno != 0) {
                    perror(NULL);
                    fclose(file);
                    return PPKG_ERROR;
                }
            }
        }

        fclose(file);
    }

    if (latestReleaseName == NULL) {
        latestReleaseName = (char*)"0.15.0+b5148c3e8fdbadc64120a0d88aae095cd5324a57";
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

    size_t tarballFileNameLength = strlen(latestReleaseVersion) + strlen(osType) + strlen(osArch) + 15U + 5U;
    char   tarballFileName[tarballFileNameLength];

    if (strcmp(osType, "macos") == 0) {
        char osVersion[31] = {0};

        if (sysinfo_vers(osVersion, 30) != 0) {
            return PPKG_ERROR;
        }

        int i = 0;

        for (;;) {
            char c = osVersion[i];

            if (c == '.') {
                osVersion[i] = '\0';
                break;
            }

            if (c == '\0') {
                break;
            }
        }

        const char * x;

        if (strcmp(osVersion, "10") == 0) {
            x = "10.15";
        } else if (strcmp(osVersion, "11") == 0) {
            x = "11.0";
        } else if (strcmp(osVersion, "12") == 0) {
            x = "12.0";
        } else {
            x = "13.0";
        }

        snprintf(tarballFileName, tarballFileNameLength, "uppm-%s-%s%s-%s.tar.xz", latestReleaseVersion, osType, x, osArch);
    } else {
        snprintf(tarballFileName, tarballFileNameLength, "uppm-%s-%s-%s.tar.xz", latestReleaseVersion, osType, osArch);
    }

    size_t   tarballUrlLength = tarballFileNameLength + strlen(latestReleaseName) + 55U;
    char     tarballUrl[tarballUrlLength];
    snprintf(tarballUrl, tarballUrlLength, "https://github.com/leleliu008/uppm/releases/download/%s/%s", latestReleaseName, tarballFileName);

    size_t   tarballFilePathLength = sessionDIRLength + tarballFileNameLength + 2U;
    char     tarballFilePath[tarballFilePathLength];
    snprintf(tarballFilePath, tarballFilePathLength, "%s/%s", sessionDIR, tarballFileName);

    ret = ppkg_http_fetch_to_file(tarballUrl, tarballFilePath, verbose, verbose);

    if (ret != PPKG_OK) {
        return ret;
    }

    ret = tar_extract(sessionDIR, tarballFilePath, 0, verbose, 1);

    if (ret != ARCHIVE_OK) {
        return abs(ret) + PPKG_ERROR_ARCHIVE_BASE;
    }

    //////////////////////////////////////////////////////////////////////////////////

    size_t   etcDIRLength = sessionDIRLength + 5U;
    char     etcDIR[etcDIRLength];
    snprintf(etcDIR, etcDIRLength, "%s/etc", sessionDIR);

    if (mkdir(etcDIR, S_IRWXU) != 0) {
        perror(etcDIR);
        return PPKG_ERROR;
    }

    size_t   cacertPemFilePathLength = etcDIRLength + 12U;
    char     cacertPemFilePath[cacertPemFilePathLength];
    snprintf(cacertPemFilePath, cacertPemFilePathLength, "%s/cacert.pem", etcDIR);

    ret = ppkg_http_fetch_to_file("https://curl.se/ca/cacert.pem", cacertPemFilePath, verbose, verbose);

    if (ret != PPKG_OK) {
        return ret;
    }

    if (setenv("SSL_CERT_FILE", cacertPemFilePath, 1) != 0) {
        perror("SSL_CERT_FILE");
        return PPKG_ERROR;
    }

    size_t   uppmCmdPathLength = sessionDIRLength + 10U;
    char     uppmCmdPath[uppmCmdPathLength];
    snprintf(uppmCmdPath, uppmCmdPathLength, "%s/bin/uppm", sessionDIR);

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (pid == 0) {
        execl(uppmCmdPath, uppmCmdPath, "update", NULL);
        perror(uppmCmdPath);
        exit(255);
    } else {
        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (childProcessExitStatusCode != 0) {
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

    //////////////////////////////////////////////////////////////////////////////////

    size_t   ppkgCoreDIRLength = ppkgHomeDIRLength + 6U;
    char     ppkgCoreDIR[ppkgCoreDIRLength];
    snprintf(ppkgCoreDIR, ppkgCoreDIRLength, "%s/core", ppkgHomeDIR);

    if (lstat(ppkgCoreDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            ret = ppkg_rm_r(ppkgCoreDIR, verbose);

            if (ret != PPKG_OK) {
                return ret;
            }
        } else {
            if (unlink(ppkgCoreDIR) != 0) {
                perror(ppkgCoreDIR);
                return PPKG_ERROR;
            }
        }
    }

    if (rename(sessionDIR, ppkgCoreDIR) != 0) {
        perror(ppkgCoreDIR);
        return PPKG_ERROR;
    } else {
        return PPKG_OK;
    }
}
