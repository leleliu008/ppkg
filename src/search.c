#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "core/regex/regex.h"

#include "ppkg.h"

static int package_name_callback(const char * packageName, size_t i, const void * regPattern) {
    if (regex_matched(packageName, (char*)regPattern) == 0) {
        if (i != 0) {
            printf("\n");
        }

        return ppkg_info(packageName, NULL);
    } else {
        if (errno == 0) {
            return PPKG_OK;
        } else {
            perror(NULL);
            return PPKG_ERROR;
        }
    }
}

int ppkg_search(const char * regPattern) {
    if (regPattern == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (regPattern[0] == '\0') {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    return ppkg_list_the_available_packages(package_name_callback, regPattern);
}
