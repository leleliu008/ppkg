#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <limits.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/file.h>

#include "core/sysinfo.h"

#include "uppm.h"
#include "ppkg.h"

static int uppm_formula_repo_url_of_official_core(char buf[], const size_t bufSize) {
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

    if (osVersMajor > 13) {
        osVersMajor = 13;
    }

    ret = snprintf(buf, bufSize, "https://github.com/leleliu008/uppm-package-repository-%s-%d.0-%s", osType, osVersMajor, osArch);
#elif defined (__DragonFly__)
    ret = snprintf(buf, bufSize, "https://github.com/leleliu008/uppm-package-repository-%s-%s-%s", osType, osVers, osArch);
#elif defined (__FreeBSD__)
    ret = snprintf(buf, bufSize, "https://github.com/leleliu008/uppm-package-repository-%s-%s-%s", osType, osVers, osArch);
#elif defined (__OpenBSD__)
    ret = snprintf(buf, bufSize, "https://github.com/leleliu008/uppm-package-repository-%s-%s-%s", osType, osVers, osArch);
#elif defined (__NetBSD__)
    ret = snprintf(buf, bufSize, "https://github.com/leleliu008/uppm-package-repository-%s-%s-%s", osType, osVers, osArch);
#else
    ret = snprintf(buf, bufSize, "https://github.com/leleliu008/uppm-formula-repository-%s-%s", osType, osArch);
#endif

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    } else {
        return PPKG_OK;
    }
}

static int uppm_formula_repo_sync_official_core_internal(const char * formulaRepoDIR) {
    char formulaRepoUrl[120];

    int ret = uppm_formula_repo_url_of_official_core(formulaRepoUrl, 120);

    if (ret != PPKG_OK) {
        return ret;
    }

    fprintf(stderr, "uppm updating formula repository from %s\n", formulaRepoUrl);

    ret = ppkg_git_sync(formulaRepoDIR, formulaRepoUrl, "refs/heads/master", "refs/remote/heads/master", "master", 0);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    char ts[11];

    ret = snprintf(ts, 11, "%ld", time(NULL));

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    size_t strCapacity = strlen(formulaRepoUrl) + (ret < 1) + 85U;
    char   str[strCapacity];

    ret = snprintf(str, strCapacity, "url: %s\nbranch: master\npinned: 0\nenabled: 1\ncreated: %s\nupdated: %s\n", formulaRepoUrl, ts, ts);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    int strLength = ret;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    size_t formulaRepoConfigFilePathCapacity = strlen(formulaRepoDIR) + 24U;
    char   formulaRepoConfigFilePath[formulaRepoConfigFilePathCapacity];

    ret = snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathCapacity, "%s/.uppm-formula-repo.yml", formulaRepoDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    int fd = open(formulaRepoConfigFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(formulaRepoConfigFilePath);
        return PPKG_ERROR;
    }

    ssize_t writeSize = write(fd, str, strLength);

    if (writeSize == -1) {
        perror(formulaRepoConfigFilePath);
        close(fd);
        return PPKG_ERROR;
    }

    close(fd);

    if (writeSize == strLength) {
        return PPKG_OK;
    } else {
        fprintf(stderr, "not fully written to %s\n", formulaRepoConfigFilePath);
        return PPKG_ERROR;
    }
}

int uppm_formula_repo_sync_official_core() {
    char   uppmHomeDIR[PATH_MAX];
    size_t uppmHomeDIRLength;

    int ret = uppm_home_dir(uppmHomeDIR, PATH_MAX, &uppmHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t formulaRepoDIRCapacity = uppmHomeDIRLength + 23U;
    char   formulaRepoDIR[formulaRepoDIRCapacity];

    ret = snprintf(formulaRepoDIR, formulaRepoDIRCapacity, "%s/repos.d/official-core", uppmHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    int formulaRepoDIRfd = 0;

    struct stat st;

    if (lstat(formulaRepoDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            formulaRepoDIRfd = open(formulaRepoDIR, O_DIRECTORY);

            if (formulaRepoDIRfd == -1) {
                puts("pppp");
                perror(formulaRepoDIR);
                return PPKG_ERROR;
            }

            ret = flock(formulaRepoDIRfd, LOCK_EX);

            if (ret == -1) {
                perror(formulaRepoDIR);
                close(formulaRepoDIRfd);
                return PPKG_ERROR;
            }

            size_t formulaRepoConfigFilePathCapacity = formulaRepoDIRCapacity + 24U;
            char   formulaRepoConfigFilePath[formulaRepoConfigFilePathCapacity];

            ret = snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathCapacity, "%s/.uppm-formula-repo.yml", formulaRepoDIR);

            if (ret < 0) {
                perror(NULL);
                close(formulaRepoDIRfd);
                return PPKG_ERROR;
            }

            if (stat(formulaRepoConfigFilePath, &st) == 0) {
                if (S_ISREG(st.st_mode)) {
                    UPPMFormulaRepo * formulaRepo = NULL;

                    ret = uppm_formula_repo_parse(formulaRepoConfigFilePath, &formulaRepo);

                    uppm_formula_repo_free(formulaRepo);

                    if (ret != PPKG_OK) {
                        ret = ppkg_rm_rf(formulaRepoDIR, true, false);

                        if (ret != PPKG_OK) {
                            close(formulaRepoDIRfd);
                            return ret;
                        }
                    }
                } else {
                    ret = ppkg_rm_rf(formulaRepoDIR, true, false);

                    if (ret != PPKG_OK) {
                        close(formulaRepoDIRfd);
                        return ret;
                    }
                }
            } else {
                ret = ppkg_rm_rf(formulaRepoDIR, true, false);

                if (ret != PPKG_OK) {
                    close(formulaRepoDIRfd);
                    return ret;
                }
            }
        } else {
            if (unlink(formulaRepoDIR) != 0) {
                perror(formulaRepoDIR);
                return PPKG_ERROR;
            }

            if (mkdir(formulaRepoDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(formulaRepoDIR);
                    return PPKG_ERROR;
                }
            }
        }
    } else {
        ret = ppkg_mkdir_p(formulaRepoDIR, false);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    if (formulaRepoDIRfd == 0) {
        formulaRepoDIRfd = open(formulaRepoDIR, O_DIRECTORY);

        if (formulaRepoDIRfd == -1) {
            perror(formulaRepoDIR);
            return PPKG_ERROR;
        }

        ret = flock(formulaRepoDIRfd, LOCK_EX);

        if (ret == -1) {
            perror(formulaRepoDIR);
            close(formulaRepoDIRfd);
            return PPKG_ERROR;
        }
    }

    ret = uppm_formula_repo_sync_official_core_internal(formulaRepoDIR);

    close(formulaRepoDIRfd);

    return ret;
}
