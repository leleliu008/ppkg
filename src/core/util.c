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

int get_file_type_id_from_url(const char * url) {
    if (regex_matched(url, ".*\\.zip$")) {
        return 1;
    } else if (regex_matched(url, ".*\\.tar\\.gz$")) {
        return 2;
    } else if (regex_matched(url, ".*\\.tar\\.xz$")) {
        return 3;
    } else if (regex_matched(url, ".*\\.tar\\.lz$")) {
        return 4;
    } else if (regex_matched(url, ".*\\.tar\\.bz2$")) {
        return 5;
    } else if (regex_matched(url, ".*\\.tgz$")) {
        return 2;
    } else if (regex_matched(url, ".*\\.txz$")) {
        return 3;
    } else if (regex_matched(url, ".*\\.tlz$")) {
        return 4;
    } else {
        return 0;
    }
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
