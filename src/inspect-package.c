#include <stdio.h>
#include <string.h>

#include "core/sysinfo.h"

#include "ppkg.h"

int ppkg_inspect_package(const char * package, const char * userSpecifiedTargetPlatformSpec, const char ** packageName, PPKGTargetPlatform * targetPlatform) {
    if (package == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (package[0] == '\0') {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    //////////////////////////////////////////////

    int slashIndex = -1;

    for (int i = 0; ;i++) {
        if (package[i] == '\0') {
            break;
        }

        if (package[i] == '/') {
            slashIndex = i;
            break;
        }
    }

    //////////////////////////////////////////////

    if (slashIndex == -1) {
        int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(package);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (userSpecifiedTargetPlatformSpec == NULL) {
            const char * PPKG_DEFAULT_TARGET = getenv("PPKG_DEFAULT_TARGET");

            if (PPKG_DEFAULT_TARGET == NULL || PPKG_DEFAULT_TARGET[0] == '\0') {
                char nativeOSType[15] = {0};

                if (sysinfo_type(nativeOSType, 30) != 0) {
                    return PPKG_ERROR;
                }

                char nativeOSVers[15] = {0};

                if (strcmp(nativeOSType, "linux") == 0) {
                    switch(sysinfo_libc()) {
                        case 1:  strncpy(nativeOSVers, "glibc", 5); break;
                        case 2:  strncpy(nativeOSVers, "musl",  4); break;
                        default: strncpy(nativeOSVers, "unknown",7);
                    }
                } else {
                    if (sysinfo_vers(nativeOSVers, 30) != 0) {
                        return PPKG_ERROR;
                    }
                }

                char nativeOSArch[15] = {0};

                if (sysinfo_arch(nativeOSArch, 30) != 0) {
                    return PPKG_ERROR;
                }

                size_t nativeOSTypeLen = strlen(nativeOSType);
                size_t nativeOSVersLen = strlen(nativeOSVers);
                size_t nativeOSArchLen = strlen(nativeOSArch);

                strncpy(targetPlatform->name, nativeOSType, nativeOSTypeLen + 1U);
                strncpy(targetPlatform->vers, nativeOSVers, nativeOSVersLen + 1U);
                strncpy(targetPlatform->arch, nativeOSArch, nativeOSArchLen + 1U);

                targetPlatform->nameLen = nativeOSTypeLen;
                targetPlatform->versLen = nativeOSVersLen;
                targetPlatform->archLen = nativeOSArchLen;

                (*packageName) = package;

                return PPKG_OK;
            } else {
                ret = ppkg_inspect_target_platform_spec(PPKG_DEFAULT_TARGET, targetPlatform);

                if (ret == PPKG_OK) {
                    (*packageName) = package;
                    return PPKG_OK;
                } else {
                    return PPKG_ERROR_PACKAGE_SPEC_IS_INVALID;
                }
            }
        } else {
            ret = ppkg_inspect_target_platform_spec(userSpecifiedTargetPlatformSpec, targetPlatform);

            if (ret == PPKG_OK) {
                (*packageName) = package;
                return PPKG_OK;
            } else {
                return PPKG_ERROR_PACKAGE_SPEC_IS_INVALID;
            }
        }
    }

    if (slashIndex == 0) {
        return PPKG_ERROR_PACKAGE_SPEC_IS_INVALID;
    }

    const char * p = package + slashIndex + 1;

    int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(p);

    if (ret != PPKG_OK) {
        return PPKG_ERROR_PACKAGE_SPEC_IS_INVALID;
    }

    char targetPlatformSpec[slashIndex + 1];

    strncpy(targetPlatformSpec, package, slashIndex);

    targetPlatformSpec[slashIndex] = '\0';

    ret = ppkg_inspect_target_platform_spec(targetPlatformSpec, targetPlatform);

    if (ret == PPKG_OK) {
        (*packageName) = p;
        return PPKG_OK;
    } else {
        return PPKG_ERROR_PACKAGE_SPEC_IS_INVALID;
    }
}
