#include <stdio.h>
#include <string.h>

#include "ppkg.h"
#include "core/regex/regex.h"

static int package_name_callback(const char * packageName, size_t i, const void * regPattern) {
    if (regex_matched(packageName, (char*)regPattern)) {
        if (i != 0) {
            printf("\n");
        }

        return ppkg_info(packageName, NULL);
    }

    return PPKG_OK;
}

int ppkg_search(const char * regPattern) {
    if (regPattern == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    size_t regPatternLength = strlen(regPattern);

    if (regPatternLength == 0) {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    return ppkg_list_the_available_packages(package_name_callback, regPattern);
}
