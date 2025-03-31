#include <string.h>

#include "ppkg.h"

int ppkg_get_platform_id_by_name(const char * const platformName, PPKGPlatformID * const platformID) {
    if (platformName == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (platformName[0] == '\0') {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    if (strcmp(platformName, "linux") == 0) {
        *platformID = PPKGPlatformID_linux;
    } else if (strcmp(platformName, "macos") == 0) {
        *platformID = PPKGPlatformID_macos;
    } else if (strcmp(platformName, "freebsd") == 0) {
        *platformID = PPKGPlatformID_freebsd;
    } else if (strcmp(platformName, "openbsd") == 0) {
        *platformID = PPKGPlatformID_openbsd;
    } else if (strcmp(platformName, "netbsd") == 0) {
        *platformID = PPKGPlatformID_netbsd;
    } else if (strcmp(platformName, "dragonflybsd") == 0) {
        *platformID = PPKGPlatformID_dragonflybsd;
    } else {
        return PPKG_ERROR_ARG_IS_INVALID;
    }

    return PPKG_OK;
}
