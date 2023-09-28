#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "ppkg.h"

int ppkg_uninstall(const char * packageName, bool verbose) {
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

    size_t   packageInstalledRootDIRLength = ppkgHomeDIRLength + strlen(packageName) + 11U;
    char     packageInstalledRootDIR[packageInstalledRootDIRLength];
    snprintf(packageInstalledRootDIR, packageInstalledRootDIRLength, "%s/installed", ppkgHomeDIR);

    size_t   packageInstalledLinkDIRLength = packageInstalledRootDIRLength + strlen(packageName) + 2U;
    char     packageInstalledLinkDIR[packageInstalledLinkDIRLength];
    snprintf(packageInstalledLinkDIR, packageInstalledLinkDIRLength, "%s/%s", packageInstalledRootDIR, packageName);

    struct stat st;

    if (lstat(packageInstalledLinkDIR, &st) == 0) {
        if (S_ISLNK(st.st_mode)) {
            size_t   receiptFilePathLength = packageInstalledLinkDIRLength + 20U;
            char     receiptFilePath[receiptFilePathLength];
            snprintf(receiptFilePath, receiptFilePathLength, "%s/.ppkg/RECEIPT.yml", packageInstalledLinkDIR);

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

                size_t   packageInstalledRealDIRLength = packageInstalledRootDIRLength + 66U;
                char     packageInstalledRealDIR[packageInstalledRealDIRLength];
                snprintf(packageInstalledRealDIR, packageInstalledRealDIRLength, "%s/%s", packageInstalledRootDIR, buf);

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
        }
    } else {
        return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }
}
