#include <string.h>

#include "ppkg.h"

int ppkg_examine_file_extension_from_url(char buf[], size_t maxSize, const char * url) {
    if (url == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (buf == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (maxSize == 0U) {
        return PPKG_ERROR_ARG_IS_INVALID;
    }

    size_t urlLength = 0U;

    for (;;) {
        if ((url[urlLength] == '?') || (url[urlLength] == '\0')) {
            break;
        } else {
            urlLength++;
        }
    }

    //printf("url=%s\nurlLength=%lu\n", url, urlLength);

    if (urlLength < 3U) {
        return PPKG_ERROR_ARG_IS_INVALID;
    }

    size_t lastIndex = urlLength - 1U;

    if (url[lastIndex] == '.') {
        return PPKG_ERROR_ARG_IS_INVALID;
    }

    size_t i = lastIndex;

    const char * p;

    for (;;) {
        if (url[i] == '.') {
            p = url + i;

            if (urlLength - i == 3U) {
                if (strcmp(p, ".gz") == 0) {
                    if (urlLength > 7U) {
                        if (strncmp(&url[i - 4U], ".tar", 4U) == 0) {
                            strncpy(buf, ".tgz", maxSize > 4U ? 4U : maxSize);
                            return PPKG_OK;
                        }
                    }
                } else if (strcmp(p, ".xz") == 0) {
                    if (urlLength > 7U) {
                        if (strncmp(&url[i - 4U], ".tar", 4U) == 0) {
                            strncpy(buf, ".txz", maxSize > 4U ? 4U : maxSize);
                            return PPKG_OK;
                        }
                    }
                } else if (strcmp(p, ".lz") == 0) {
                    if (urlLength > 7U) {
                        if (strncmp(&url[i - 4], ".tar", 4U) == 0) {
                            strncpy(buf, ".tlz", maxSize > 4U ? 4U : maxSize);
                            return PPKG_OK;
                        }
                    }
                }
            } else if (urlLength - i == 4U) {
                if (strcmp(p, ".bz2") == 0) {
                    if (urlLength > 8) {
                        if (strncmp(&url[i - 4U], ".tar", 4U) == 0) {
                            strncpy(buf, ".tbz2", maxSize > 5U ? 5U : maxSize);
                            return PPKG_OK;
                        }
                    }
                }
            }

            size_t n = urlLength - i;
            strncpy(buf, p, maxSize > n ? n : maxSize);
            return PPKG_OK;
        } else {
            if (i == 0) {
                return PPKG_ERROR_ARG_IS_INVALID;
            } else {
                i--;
            }
        }
    }
}
