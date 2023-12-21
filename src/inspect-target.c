#include <stdio.h>
#include <string.h>

#include "ppkg.h"

int ppkg_inspect_target_platform_spec(const char * targetPlatformSpec, PPKGTargetPlatform * targetPlatform) {
    if (targetPlatformSpec == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (targetPlatformSpec[0] == '\0') {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    //////////////////////////////////////////////

    const char * p = targetPlatformSpec;

    //////////////////////////////////////////////

    char   targetPlarformName[16];
    size_t targetPlarformNameLength;

    for (int i = 0; ;i++) {
        if (p[i] == '\0') {
            fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
            return PPKG_ERROR;
        }

        if (p[i] == '-') {
            if (i > 15) {
                fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
                return PPKG_ERROR;
            }

            const char * supportedTargetPlarformNames[6] = { "linux", "macos", "freebsd", "openbsd", "netbsd", "dragonflybsd" };

            bool isSupported = false;

            for (int j = 0; j < 6; j++) {
                if (strncmp(p, supportedTargetPlarformNames[j], i) == 0) {
                    isSupported = true;
                    break;
                }
            }

            if (isSupported) {
                targetPlarformNameLength = i;
                strncpy(targetPlarformName, p, targetPlarformNameLength);
                targetPlarformName[targetPlarformNameLength] = '\0';

                p += i + 1;

                break;
            } else {
                fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
                return PPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////

    char   targetPlarformVers[16];
    size_t targetPlarformVersLength;

    for (int i = 0; ;i++) {
        if (p[i] == '\0') {
            fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
            return PPKG_ERROR;
        }

        if (p[i] == '-') {
            if (i > 15) {
                fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
                return PPKG_ERROR;
            }

            strncpy(targetPlarformVers, p, i);
            targetPlarformVers[i] = '\0';

            targetPlarformVersLength = i;

            p += i + 1;

            break;
        }
    }

    //////////////////////////////////////////////

    char   targetPlarformArch[16];
    size_t targetPlarformArchLength;

    for (int i = 0; ;i++) {
        if (p[i] == '-') {
            fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
            return PPKG_ERROR;
        }

        if (p[i] == '\0') {
            if (i > 15) {
                fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
                return PPKG_ERROR;
            }

            strncpy(targetPlarformArch, p, i);
            targetPlarformArch[i] = '\0';

            targetPlarformArchLength = i;

            break;
        }
    }

    //////////////////////////////////////////////

    strncpy(targetPlatform->name, targetPlarformName, targetPlarformNameLength + 1U);
    strncpy(targetPlatform->vers, targetPlarformVers, targetPlarformVersLength + 1U);
    strncpy(targetPlatform->arch, targetPlarformArch, targetPlarformArchLength + 1U);

    targetPlatform->nameLen = targetPlarformNameLength;
    targetPlatform->versLen = targetPlarformVersLength;
    targetPlatform->archLen = targetPlarformArchLength;

    return PPKG_OK;
}
