#include <math.h>
#include <errno.h>
#include <string.h>

#include <limits.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "core/regex/regex.h"
#include "core/sysinfo.h"
#include "core/http.h"
#include "core/tar.h"
#include "core/log.h"

#include "ppkg.h"

static int ppkg_core_tar_filename(char buf[], const size_t bufSize, const char * latestVersion) {
    char osType[31] = {0};

    if (sysinfo_type(osType, 30) != 0) {
        return PPKG_ERROR;
    }

    char osArch[31] = {0};

    if (sysinfo_arch(osArch, 30) != 0) {
        return PPKG_ERROR;
    }

    char osVers[31] = {0};

    if (sysinfo_vers(osVers, 30) != 0) {
        return PPKG_ERROR;
    }

    int ret;

#if defined (__APPLE__)
    int osVersMajor = 0;

    for (int i = 0; i < 31; i++) {
        if (osVers[i] == '\0') {
            break;
        }

        if (osVers[i] == '.') {
            osVers[i] = '\0';
            osVersMajor = atoi(osVers);
            break;
        }
    }

    if (osVersMajor < 10) {
        fprintf(stderr, "MacOSX %d.x is not supported.\n", osVersMajor);
        return PPKG_ERROR;
    }

    if (osVersMajor > 14) {
        osVersMajor = 14;
    }

    ret = snprintf(buf, bufSize, "ppkg-core-%s-%s-%d.0-%s.tar.xz", latestVersion, osType, osVersMajor, osArch);
#elif defined (__DragonFly__)
    ret = snprintf(buf, bufSize, "ppkg-core-%s-%s-%s-%s.tar.xz", latestVersion, osType, osVers, osArch);
#elif defined (__FreeBSD__)
    double v = atof(osVers) * 10;

    if (v < 130) {
        osVers[0] = '1';
        osVers[1] = '3';
        osVers[2] = '.';
        osVers[3] = '0';
        osVers[4] = '\0';
    } else if (v > 132) {
        osVers[0] = '1';
        osVers[1] = '3';
        osVers[2] = '.';
        osVers[3] = '2';
        osVers[4] = '\0';
    }

    ret = snprintf(buf, bufSize, "ppkg-core-%s-%s-%s-%s.tar.xz", latestVersion, osType, osVers, osArch);
#elif defined (__OpenBSD__)
    double v = atof(osVers) * 10;

    if (v < 73) {
        osVers[0] = '7';
        osVers[1] = '.';
        osVers[2] = '3';
        osVers[3] = '\0';
    } else if (v > 74) {
        osVers[0] = '7';
        osVers[1] = '.';
        osVers[2] = '4';
        osVers[3] = '\0';
    }

    ret = snprintf(buf, bufSize, "ppkg-core-%s-%s-%s-%s.tar.xz", latestVersion, osType, osVers, osArch);
#elif defined (__NetBSD__)
    double v = atof(osVers) * 10;

    if (v < 92) {
        osVers[0] = '9';
        osVers[1] = '.';
        osVers[2] = '2';
        osVers[3] = '\0';
    } else if (v > 93) {
        osVers[0] = '9';
        osVers[1] = '.';
        osVers[2] = '3';
        osVers[3] = '\0';
    }

    ret = snprintf(buf, bufSize, "ppkg-core-%s-%s-%s-%s.tar.xz", latestVersion, osType, osVers, osArch);
#else
    ret = snprintf(buf, bufSize, "ppkg-core-%s-%s-%s.tar.xz", latestVersion, osType, osArch);
#endif

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    } else {
        return PPKG_OK;
    }
}

int ppkg_setup(const bool verbose) {
    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    int ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    struct stat st;

    size_t ppkgRunDIRCapacity = ppkgHomeDIRLength + 5U;
    char   ppkgRunDIR[ppkgRunDIRCapacity];

    ret = snprintf(ppkgRunDIR, ppkgRunDIRCapacity, "%s/run", ppkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

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

    size_t sessionDIRCapacity = ppkgRunDIRCapacity + 20U;
    char   sessionDIR[sessionDIRCapacity];

    ret = snprintf(sessionDIR, sessionDIRCapacity, "%s/%d", ppkgRunDIR, getpid());

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (lstat(sessionDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            ret = ppkg_rm_rf(sessionDIR, false, verbose);

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

    if (chdir(sessionDIR) != 0) {
        perror(sessionDIR);
        return PPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    ret = ppkg_http_fetch_to_file("https://raw.githubusercontent.com/leleliu008/ppkg/c/ppkg-install", "ppkg-install", verbose, verbose);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    ret = ppkg_http_fetch_to_file("https://raw.githubusercontent.com/leleliu008/ppkg/c/ppkg-do12345", "ppkg-do12345", verbose, verbose);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    const char * const ppkgCoreLatestVersion = "2024.06.30";

    size_t ppkgCoreTarballFilenameCapacity = 100U;
    char   ppkgCoreTarballFilename[ppkgCoreTarballFilenameCapacity];

    ret = ppkg_core_tar_filename(ppkgCoreTarballFilename, ppkgCoreTarballFilenameCapacity, ppkgCoreLatestVersion);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////////

    size_t urlCapacity = 256U;
    char   url[urlCapacity];

    ret = snprintf(url, urlCapacity, "https://github.com/leleliu008/ppkg/releases/download/ppkg-core-%s/%s", ppkgCoreLatestVersion, ppkgCoreTarballFilename);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    ret = ppkg_http_fetch_to_file(url, ppkgCoreTarballFilename, verbose, verbose);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////////

    ret = tar_extract(sessionDIR, ppkgCoreTarballFilename, ARCHIVE_EXTRACT_TIME, verbose, 1);

    if (ret != 0) {
        return abs(ret) + PPKG_ERROR_ARCHIVE_BASE;
    }

    //////////////////////////////////////////////////////////////////////////////////

    ret = ppkg_http_fetch_to_file("https://curl.se/ca/cacert.pem", "cacert.pem", verbose, verbose);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    if (chdir(ppkgRunDIR) != 0) {
        perror(ppkgRunDIR);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////

    size_t ppkgCoreDIRCapacity = ppkgHomeDIRLength + 6U;
    char   ppkgCoreDIR[ppkgCoreDIRCapacity];

    ret = snprintf(ppkgCoreDIR, ppkgCoreDIRCapacity, "%s/core", ppkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    for (;;) {
        if (rename(sessionDIR, ppkgCoreDIR) == 0) {
            return PPKG_OK;
        } else {
            if (errno == ENOTEMPTY || errno == EEXIST) {
                if (lstat(ppkgCoreDIR, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        ret = ppkg_rm_rf(ppkgCoreDIR, false, verbose);

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
            } else {
                perror(ppkgCoreDIR);
                return PPKG_ERROR;
            }
        }
    }
}
