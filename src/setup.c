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

    const char* items[8] = { "wrapper-native-c++.c", "wrapper-native-objc.c", "wrapper-target-cc.c", "wrapper-target-clang.c", "wrapper-native-cc.c", "wrapper-target-c++.c", "wrapper-target-clang++.c", "wrapper-target-objc.c" };

    for (int i = 0; i < 8; i++) {
        const char * item = items[i];

        size_t urlCapacity = strlen(item) + 53U;
        char   url[urlCapacity];

        ret = snprintf(url, urlCapacity, "https://raw.githubusercontent.com/leleliu008/ppkg/c/%s", item);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        ret = ppkg_http_fetch_to_file(url, item, verbose, verbose);

        if (ret != PPKG_OK) {
            return ret;
        }
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
