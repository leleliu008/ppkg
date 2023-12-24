#include <stdio.h>
#include <string.h>

#include "ppkg.h"

static const char * supportedTargetPlatformNames[6] = { "linux", "macos", "freebsd", "openbsd", "netbsd", "dragonflybsd" };

int ppkg_inspect_target_platform_spec(const char * targetPlatformSpec, PPKGTargetPlatform * targetPlatform) {
    if (targetPlatformSpec == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (targetPlatformSpec[0] == '\0') {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    //////////////////////////////////////////////

    const char * p = targetPlatformSpec;

    int k = -1;

    //////////////////////////////////////////////

    char   targetPlatformName[16];
    size_t targetPlatformNameLength;

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

            for (int j = 0; j < 6; j++) {
                if (strncmp(p, supportedTargetPlatformNames[j], i) == 0) {
                    k = j;
                    break;
                }
            }

            if (k == -1) {
                fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
                return PPKG_ERROR;
            } else {
                targetPlatformNameLength = i;
                strncpy(targetPlatformName, p, targetPlatformNameLength);
                targetPlatformName[targetPlatformNameLength] = '\0';

                p += i + 1;

                break;
            }
        }
    }

    //////////////////////////////////////////////

    char   targetPlatformVers[6];
    size_t targetPlatformVersLength;

    for (int i = 0; ;i++) {
        if (p[i] == '\0') {
            fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
            return PPKG_ERROR;
        }

        if (p[i] == '-') {
            if (i > 5) {
                fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
                return PPKG_ERROR;
            }

            strncpy(targetPlatformVers, p, i);
            targetPlatformVers[i] = '\0';

            targetPlatformVersLength = i;

            // linux
            if (k == 0) {
                if (!(strcmp(targetPlatformVers, "glibc") == 0 || strcmp(targetPlatformVers, "musl") == 0)) {
                    fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
                    return PPKG_ERROR;
                }
            } else {
                int dotIndex = -1;

                for (int m = 0; ;m++) {
                    if (targetPlatformVers[m] == '\0') {
                        break;
                    }

                    if (targetPlatformVers[m] == '.') {
                        if (dotIndex == -1) {
                            if (m == 0 || m == ((int)targetPlatformVersLength - 1)) {
                                fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
                                return PPKG_ERROR;
                            } else {
                                dotIndex = m;
                            }
                        } else {
                            fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
                            return PPKG_ERROR;
                        }
                    } else {
                        if (targetPlatformVers[m] < '0' || targetPlatformVers[m] > '9') {
                            fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
                            return PPKG_ERROR;
                        }
                    }
                }
            }

            p += i + 1;

            break;
        }
    }

    //////////////////////////////////////////////

    char   targetPlatformArch[16];
    size_t targetPlatformArchLength;

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

            strncpy(targetPlatformArch, p, i);
            targetPlatformArch[i] = '\0';

            targetPlatformArchLength = i;

            break;
        }
    }

    //////////////////////////////////////////////

    strncpy(targetPlatform->name, targetPlatformName, targetPlatformNameLength + 1U);
    strncpy(targetPlatform->vers, targetPlatformVers, targetPlatformVersLength + 1U);
    strncpy(targetPlatform->arch, targetPlatformArch, targetPlatformArchLength + 1U);

    targetPlatform->nameLen = targetPlatformNameLength;
    targetPlatform->versLen = targetPlatformVersLength;
    targetPlatform->archLen = targetPlatformArchLength;

    return PPKG_OK;
}
