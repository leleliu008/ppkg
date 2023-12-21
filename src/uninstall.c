#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "ppkg.h"

int ppkg_uninstall(const char * packageName, const PPKGTargetPlatform * targetPlatform, bool verbose) {
    int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t packageInstalledRootDIRCapacity = ppkgHomeDIRLength + targetPlatform->nameLen + targetPlatform->versLen + targetPlatform->archLen + 14U;
    char   packageInstalledRootDIR[packageInstalledRootDIRCapacity];

    ret = snprintf(packageInstalledRootDIR, packageInstalledRootDIRCapacity, "%s/installed/%s-%s-%s", ppkgHomeDIR, targetPlatform->name, targetPlatform->vers, targetPlatform->arch);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    size_t packageInstalledLinkDIRCapacity = packageInstalledRootDIRCapacity + strlen(packageName) + 2U;
    char   packageInstalledLinkDIR[packageInstalledLinkDIRCapacity];

    ret = snprintf(packageInstalledLinkDIR, packageInstalledLinkDIRCapacity, "%s/%s", packageInstalledRootDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    struct stat st;

    if (lstat(packageInstalledLinkDIR, &st) == 0) {
        if (S_ISLNK(st.st_mode)) {
            size_t receiptFilePathCapacity = packageInstalledLinkDIRCapacity + 20U;
            char   receiptFilePath[receiptFilePathCapacity];

            ret = snprintf(receiptFilePath, receiptFilePathCapacity, "%s/.ppkg/RECEIPT.yml", packageInstalledLinkDIR);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            if (lstat(receiptFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
                char buf[256] = {0};

                ssize_t readSize = readlink(packageInstalledLinkDIR, buf, 255);

                if (readSize == -1) {
                    perror(packageInstalledLinkDIR);
                    return PPKG_ERROR;
                } else if (readSize != 64) {
                    // package is broken by other tools?
                    return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
                }

                size_t packageInstalledRealDIRCapacity = packageInstalledRootDIRCapacity + 66U;
                char   packageInstalledRealDIR[packageInstalledRealDIRCapacity];

                ret = snprintf(packageInstalledRealDIR, packageInstalledRealDIRCapacity, "%s/%s", packageInstalledRootDIR, buf);

                if (ret < 0) {
                    perror(NULL);
                    return PPKG_ERROR;
                }

                if (lstat(packageInstalledRealDIR, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        if (unlink(packageInstalledLinkDIR) == 0) {
                            if (verbose) {
                                printf("rm %s\n", packageInstalledLinkDIR);
                            }
                        } else {
                            perror(packageInstalledLinkDIR);
                            return PPKG_ERROR;
                        }

                        return ppkg_rm_r(packageInstalledRealDIR, verbose);
                    } else {
                        // package is broken by other tools?
                        return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
                    }
                } else {
                    // package is broken by other tools?
                    return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
                }
            } else {
                // package is broken. is not installed completely?
                return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
            }
        } else {
            return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
        }
    } else {
        return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }
}
