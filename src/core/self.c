#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/dyld.3.html
#if defined (__APPLE__)
#include <mach-o/dyld.h>
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

#include "self.h"

int selfpath(char buf[]) {
#if defined (__APPLE__)
    // https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/dyld.3.html
    uint32_t bufSize = 0U;
    _NSGetExecutablePath(NULL, &bufSize);

    char path[bufSize];
    _NSGetExecutablePath(path, &bufSize);

    realpath(path, buf);

    return 0;
#elif defined (__FreeBSD__)
    const int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };

    size_t len;

    if (sysctl(mib, 4, NULL, &len, NULL, 0) < 0) {
        return -1;
    }

    if (sysctl(mib, 4,  buf, &len, NULL, 0) < 0) {
        return -1;
    }

    return 0;
#elif defined (__OpenBSD__)
    const int mib[4] = { CTL_KERN, KERN_PROC_ARGS, getpid(), KERN_PROC_ARGV };

    size_t len;

    if (sysctl(mib, 4, NULL, &len, NULL, 0) != 0) {
        return -1;
    }

    char** argv = (char**)calloc(len, sizeof(char*));

    if (argv == NULL) {
        errno = ENOMEM;
        return -1;
    }

    if (sysctl(mib, 4, argv, &size, NULL, 0) != 0) {
        free(argv);
        return -1;
    }

    char * p = argv[0];

    int ispath;

    for (;;) {
        if (p[0] == '\0') {
            ispath = 0;
            break;
        }

        if (p[0] == '/') {
            ispath = 1;
            break;
        }

        p++;
    }

    if (ispath == 1) {
        realpath(p, buf);
        free(argv);
        return 0;
    } else {
        struct stat st;

        char tmpBuf[PATH_MAX];
        char outBuf[PATH_MAX];

        const char * p = getenv("PATH");

        while (p != NULL) {
            for (int i = 0;; i++) {
                if (p[i] == '\0') {
                    p = NULL;
                    tmpBuf[i] = '\0';

                    if (i != 0) {
                        if ((stat(tmpBuf, &st) == 0) && S_ISDIR(st.st_mode)) {
                            int n = snprintf(outBuf, PATH_MAX, "%s/%s", tmpBuf, commandName);

                            if (n < 0) {
                                return -1;
                            }

                            if (access(outBuf, X_OK) == 0) {
                                strncpy(buf, outBuf, n);

                                buf[n] = '\0';

                                return n;
                            }
                        }
                    }

                    break;
                }

                if (p[i] == ':') {
                    p += i + 1;
                    tmpBuf[i] = '\0';

                    if (i != 0) {
                        if ((stat(tmpBuf, &st) == 0) && S_ISDIR(st.st_mode)) {
                            int n = snprintf(outBuf, PATH_MAX, "%s/%s", tmpBuf, commandName);

                            if (n < 0) {
                                return -1;
                            }

                            if (access(outBuf, X_OK) == 0) {
                                strncpy(buf, outBuf, n);

                                buf[n] = '\0';

                                return n;
                            }
                        }
                    }

                    break;
                }

                tmpBuf[i] = p[i];
            }
        }

        free(argv);
        return 0;
    }
#else
    ssize_t n = readlink("/proc/self/exe", buf, PATH_MAX - 1U);

    if (n == -1) {
        return -1;
    } else {
        buf[n] = '\0';
        return 0;
    }
#endif
}
