#include <regex.h>
#include "regex/regex.h"
#include "util.h"
#include "find-executables.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

#if defined (__APPLE__)
#include <sys/syslimits.h>
#elif defined (__linux__) && defined (HAVE_LINUX_LIMITS_H)
#include <linux/limits.h>
#else
#include <limits.h>
#endif

#if defined (__FreeBSD__) || defined (__OpenBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

int get_file_extension_from_url(char * * out, const char * url) {
    if (url == NULL) {
        return -1;
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
        return -2;
    }

    size_t lastIndex = urlLength - 1;

    if (url[lastIndex] == '.') {
        return -3;
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
                            (*out) = strdup(".tgz");
                            return 0;
                        }
                    }
                } else if (strcmp(p, ".xz") == 0) {
                    if (urlLength > 7) {
                        if (strncmp(&url[i - 4], ".tar", 4) == 0) {
                            (*out) = strdup(".txz");
                            return 0;
                        }
                    }
                } else if (strcmp(p, ".lz") == 0) {
                    if (urlLength > 7) {
                        if (strncmp(&url[i - 4], ".tar", 4) == 0) {
                            (*out) = strdup(".tlz");
                            return 0;
                        }
                    }
                }
            } else if (urlLength - i == 4) {
                if (strcmp(p, ".bz2") == 0) {
                    if (urlLength > 8) {
                        if (strncmp(&url[i - 4], ".tar", 4) == 0) {
                            (*out) = strdup(".tbz2");
                            return 0;
                        }
                    }
                }
            }

            (*out) = strndup(p, urlLength - i);
            return 0;
        }

        if (i == 0) {
            break;
        }
    }

    return -3;
}

int get_current_executable_realpath(char * * out) {
#if defined (__APPLE__)
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
#elif defined (__FreeBSD__)
    const int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };

    size_t bufLength = 0;

    if (sysctl(mib, 4, NULL, &bufLength, NULL, 0) < 0) {
        return -1;
    }

    char * buf = (char*)calloc(bufLength + 1, sizeof(char));

    if (sysctl(mib, 4, buf, &bufLength, NULL, 0) < 0) {
        return -2;
    }

    (*out) = buf;
    return 0;
#elif defined (__OpenBSD__)
    const int mib[4] = { CTL_KERN, KERN_PROC_ARGS, getpid(), KERN_PROC_ARGV };
    size_t size;

    if (sysctl(mib, 4, NULL, &size, NULL, 0) != 0) {
        return -1;
    }

    char** argv = (char**)malloc(size);
    memset(argv, 0, size);

    if (sysctl(mib, 4, argv, &size, NULL, 0) != 0) {
        return -2;
    }

    bool isPath = false;

    char c;

    char * p = argv[0];

    for (;;) {
        c = p[0];

        if (c == '\0') {
            break;
        }

        if (c == '/') {
            isPath = true;
            break;
        }

        p++;
    }

    if (isPath) {
        (*out) = realpath(argv[0], NULL);
        return 0;
    } else {
        char * PATH = getenv("PATH");

        if ((PATH == NULL) || (strcmp(PATH, "") == 0)) {
            return -3;
        }

        size_t PATH2Length = strlen(PATH) + 1;
        char   PATH2[PATH2Length];
        memset(PATH2, 0, PATH2Length);
        strcpy(PATH2, PATH);

        size_t commandNameLength = strlen(argv[0]);

        char * PATHItem = strtok(PATH2, ":");

        while (PATHItem != NULL) {
            struct stat st;

            if ((stat(PATHItem, &st) == 0) && S_ISDIR(st.st_mode)) {
                size_t  fullPathLength = strlen(PATHItem) + commandNameLength + 2;
                char    fullPath[fullPathLength];
                memset( fullPath, 0, fullPathLength);
                sprintf(fullPath, "%s/%s", PATHItem, argv[0]);

                if (access(fullPath, X_OK) == 0) {
                    (*out) = strdup(fullPath);
                    return 0;
                }
            }

            PATHItem = strtok(NULL, ":");
        }

        return -4;
    }
#else
    char buf[PATH_MAX + 1] = {0};

    int ret = readlink("/proc/self/exe", buf, PATH_MAX);

    if (ret == -1) {
        perror("/proc/self/exe");
        return -1;
    }

    (*out) = strdup(buf);
    return 0;
#endif
}
