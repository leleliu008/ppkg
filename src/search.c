#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "core/regex/regex.h"

#include "ppkg.h"

static size_t j = 0U;

static int package_name_filter(const char * packageName, const char * targetPlatformName, const bool verbose, size_t i, const void * regPattern) {
    if (regex_matched(packageName, (char*)regPattern) == 0) {
        if (verbose) {
            if (j != 0U) {
                printf("\n");
            }

            j++;

            return ppkg_available_info(packageName, targetPlatformName, NULL);
        } else {
            puts(packageName);
            return PPKG_OK;
        }
    } else {
        if (errno == 0) {
            return PPKG_OK;
        } else {
            perror(NULL);
            return PPKG_ERROR;
        }
    }
}

int ppkg_search(const char * regPattern, const char * targetPlatformName, const bool verbose) {
    if (regPattern == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (regPattern[0] == '\0') {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    return ppkg_list_the_available_packages(targetPlatformName, verbose, package_name_filter, regPattern);
}
