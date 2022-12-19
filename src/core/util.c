#include <regex.h>
#include "regex/regex.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined (__APPLE__)
#include <sys/syslimits.h>
#elif defined (__linux__) && defined (HAVE_LINUX_LIMITS_H)
#include <linux/limits.h>
#else
#include <limits.h>
#endif

int get_file_extension_from_url(char * * out, const char * url) {
    if (url == NULL) {
        return -1;
    }

    size_t urlLength = strlen(url);

    if (urlLength < 3) {
        return -2;
    }

    size_t lastIndex = urlLength - 1;

    if (url[lastIndex] == '.') {
        return -3;
    }

    size_t i = lastIndex;

    char c;

    const char * p;

    for (; i >= 0; i--) {
        c = url[i];

        if (c == '.') {
            p = &url[i];

            if (urlLength - i == 3) {
                if (strcmp(p, ".gz") == 0 || strcmp(p, ".xz") == 0 || strcmp(p, ".lz") == 0) {
                    if (urlLength > 7) {
                        const char * p2 = &url[i - 4];

                        if (strncmp(p2, ".tar", 4) == 0) {
                            (*out) = strdup(p2);
                            return 0;
                        }
                    }
                }
            } else if (urlLength - i == 4) {
                if (strcmp(p, ".bz2") == 0) {
                    if (urlLength > 8) {
                        const char * p2 = &url[i - 4];

                        if (strncmp(p2, ".tar", 4) == 0) {
                            (*out) = strdup(p2);
                            return 0;
                        }
                    }
                }
            }

            (*out) = strdup(p);
            return 0;
        }

        if (i == 0) {
            break;
        }
    }

    return -3;
}

int get_current_executable_realpath(char * * out) {
#if defined (__linux__)
    char buf[PATH_MAX + 1] = {0};

    int ret = readlink("/proc/self/exe", buf, PATH_MAX);

    if (ret == -1) {
        perror("/proc/self/exe");
        return -1;
    }

    (*out) = strdup(buf);
    return 0;
#elif defined (__APPLE__)
    char buf[PATH_MAX + 1] = {0};

    uint32_t bufSize = 0U;
    _NSGetExecutablePath(NULL, &bufSize);

    char path[bufSize];
    _NSGetExecutablePath(path, &bufSize);

    if (realpath(path, buf) == NULL) {
        (*out) = NULL;
        return -1;
    }

    (*out) = strdup(buf);
    return 0;
#else
    (*out) = NULL;
    return -1;
#endif
}
