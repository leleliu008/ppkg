#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "core/regex/regex.h"

#include "ppkg.h"

static size_t j = 0U;

static int package_name_callback(const char * packageName, const char * targetPlatformName, size_t i, const void * regPattern) {
    if (regex_matched(packageName, (char*)regPattern) == 0) {
        if (j != 0) {
            printf("\n");
        }

        j++;

        return ppkg_available_info(packageName, targetPlatformName, NULL);
    } else {
        if (errno == 0) {
            return PPKG_OK;
        } else {
            perror(NULL);
            return PPKG_ERROR;
        }
    }
}

int ppkg_search(const char * regPattern, const char * targetPlatformName) {
    if (regPattern == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (regPattern[0] == '\0') {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    return ppkg_list_the_available_packages(targetPlatformName, package_name_callback, regPattern);
}
