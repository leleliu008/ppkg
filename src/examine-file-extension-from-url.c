#include <string.h>

#include "ppkg.h"

int ppkg_examine_file_extension_from_url(char buf[], size_t maxSize, const char * url) {
    if (url == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (buf == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (maxSize == 0) {
        return PPKG_ERROR_ARG_IS_INVALID;
    }

    size_t urlLength = 0;
    char c;

    for (;;) {
        c = url[urlLength];

        if (c == '?' || c == '\0') {
            break;
        } else {
            urlLength++;
        }
    }

    //printf("url=%s\nurlLength=%lu\n", url, urlLength);

    if (urlLength < 3) {
        return PPKG_ERROR_ARG_IS_INVALID;
    }

    size_t lastIndex = urlLength - 1;

    if (url[lastIndex] == '.') {
        return PPKG_ERROR_ARG_IS_INVALID;
    }

    size_t i = lastIndex;

    const char * p;

    for (; i >= 0; i--) {
        c = url[i];

        if (c == '.') {
            p = &url[i];

            if (urlLength - i == 3) {
                if (strcmp(p, ".gz") == 0) {
                    if (urlLength > 7) {
                        if (strncmp(&url[i - 4], ".tar", 4) == 0) {
                            strncpy(buf, ".tgz", maxSize > 4 ? 4 : maxSize);
                            return PPKG_OK;
                        }
                    }
                } else if (strcmp(p, ".xz") == 0) {
                    if (urlLength > 7) {
                        if (strncmp(&url[i - 4], ".tar", 4) == 0) {
                            strncpy(buf, ".txz", maxSize > 4 ? 4 : maxSize);
                            return PPKG_OK;
                        }
                    }
                } else if (strcmp(p, ".lz") == 0) {
                    if (urlLength > 7) {
                        if (strncmp(&url[i - 4], ".tar", 4) == 0) {
                            strncpy(buf, ".tlz", maxSize > 4 ? 4 : maxSize);
                            return PPKG_OK;
                        }
                    }
                }
            } else if (urlLength - i == 4) {
                if (strcmp(p, ".bz2") == 0) {
                    if (urlLength > 8) {
                        if (strncmp(&url[i - 4], ".tar", 4) == 0) {
                            strncpy(buf, ".tbz2", maxSize > 5 ? 5 : maxSize);
                            return PPKG_OK;
                        }
                    }
                }
            }

            size_t n = urlLength - i;
            strncpy(buf, p, maxSize > n ? n : maxSize);
            return PPKG_OK;
        }

        if (i == 0) {
            break;
        }
    }

    return PPKG_ERROR_ARG_IS_INVALID;
}
