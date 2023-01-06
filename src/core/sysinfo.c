#include "sysinfo.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include "regex/regex.h"

#include <unistd.h>

int sysinfo_kind(char * buf, size_t bufSize) {
#if defined (_WIN32)
    strncpy(buf, "windows", bufSize > 7 ? 7 : bufSize);
    return 0;
#elif defined (__APPLE__)
    strncpy(buf, "darwin", bufSize > 6 ? 6 : bufSize);
    return 0;
#elif defined (__FreeBSD__)
    strncpy(buf, "freebsd", bufSize > 7 ? 7 : bufSize);
    return 0;
#elif defined (__OpenBSD__)
    strncpy(buf, "openbsd", bufSize > 7 ? 7 : bufSize);
    return 0;
#elif defined (__NetBSD__)
    strncpy(buf, "netbsd", bufSize > 6 ? 6 : bufSize);
    return 0;
#elif defined (__ANDROID__)
    strncpy(buf, "android", bufSize > 7 ? 7 : bufSize);
    return 0;
#elif defined (__linux__)
    strncpy(buf, "linux", bufSize > 5 ? 5 : bufSize);
    return 0;
#else
    struct utsname uts;

    if (uname(&uts) < 0) {
        perror("uname() error");
        return -1;
    }

    size_t osKindLength = strlen(uts.sysname);
    size_t n = bufSize > osKindLength ? osKindLength : bufSize;

    strncpy(buf, uts.sysname, n);

    for (size_t i = 0; i < n; i++) {
        if ((buf[i] >= 'A') && (buf[i] <= 'Z')) {
             buf[i] += 32;
        }
    }

    return 0;
#endif
}

int sysinfo_type(char * buf, size_t bufSize) {
#if defined (_WIN32)
    strncpy(buf, "windows", bufSize > 7 ? 7 : bufSize);
    return 0;
#elif defined (__APPLE__)
    strncpy(buf, "macos", bufSize > 5 ? 5 : bufSize);
    return 0;
#elif defined (__FreeBSD__)
    strncpy(buf, "freebsd", bufSize > 7 ? 7 : bufSize);
    return 0;
#elif defined (__OpenBSD__)
    strncpy(buf, "openbsd", bufSize > 7 ? 7 : bufSize);
    return 0;
#elif defined (__NetBSD__)
    strncpy(buf, "netbsd", bufSize > 6 ? 6 : bufSize);
    return 0;
#elif defined (__ANDROID__)
    strncpy(buf, "android", bufSize > 7 ? 7 : bufSize);
    return 0;
#elif defined (__linux__)
    strncpy(buf, "linux", bufSize > 5 ? 5 : bufSize);
    return 0;
#else
    struct utsname uts;

    if (uname(&uts) < 0) {
        perror("uname() error");
        return -1;
    }

    size_t osKindLength = strlen(uts.sysname);
    size_t n = bufSize > osKindLength ? osKindLength : bufSize;

    strncpy(buf, uts.sysname, n);

    for (size_t i = 0; i < n; i++) {
        if ((buf[i] >= 'A') && (buf[i] <= 'Z')) {
             buf[i] += 32;
        }
    }

    return 0;
#endif
}

int sysinfo_arch(char * buf, size_t bufSize) {
    struct utsname uts;

    if (uname(&uts) < 0) {
        perror("uname() error");
        return -1;
    }

    size_t osArchLength = strlen(uts.machine);

    strncpy(buf, uts.machine, bufSize > osArchLength ? osArchLength : bufSize);

    return 0;
}

int sysinfo_name(char * buf, size_t bufSize) {
#if defined (_WIN32)
    strncpy(buf, "windows", bufSize > 7 ? 7 : bufSize);
    return 0;
#elif defined (__APPLE__)
    strncpy(buf, "macos", bufSize > 5 ? 5 : bufSize);
    return 0;
#elif defined (__FreeBSD__)
    strncpy(buf, "freebsd", bufSize > 7 ? 7 : bufSize);
    return 0;
#elif defined (__OpenBSD__)
    strncpy(buf, "openbsd", bufSize > 7 ? 7 : bufSize);
    return 0;
#elif defined (__NetBSD__)
    strncpy(buf, "netbsd", bufSize > 6 ? 6 : bufSize);
    return 0;
#elif defined (__ANDROID__)
    strncpy(buf, "android", bufSize > 7 ? 7 : bufSize);
    return 0;
#else
    struct stat sb;

    if ((stat("/etc/os-release", &sb) == 0) && (S_ISREG(sb.st_mode) || S_ISLNK(sb.st_mode))) {
        FILE * file = fopen("/etc/os-release", "r");

        if (file == NULL) {
            perror("/etc/os-release");
            return -1;
        }

        char line[50];

        while (fgets(line, 50, file) != NULL) {
            if (regex_matched(line, "^ID=.*")) {
                char * p = &line[3];

                if (p[0] == '"' || p[0] == '\'') {
                    p++;
                }

                size_t n = strlen(p);

                p[n - 1] = '\0';
                n--;

                if (p[n - 1] == '"' || p[n - 1] == '\'') {
                    p[n - 1] = '\0';
                    n--;
                }

                strncpy(buf, p, bufSize > n ? n : bufSize);
                return 0;
            }
        }

        fclose(file);
    }

    return -2;
#endif
}

int sysinfo_vers(char * buf, size_t bufSize) {
#if defined (__NetBSD__) || defined (__OpenBSD__)
    struct utsname uts;

    if (uname(&uts) < 0) {
        perror("uname() error");
        return -1;
    }

    size_t n = strlen(uts.release);

    strncpy(buf, uts.release, bufSize > n ? n : bufSize);

    return 0;
#elif defined (__APPLE__)
    const char * filepath = "/System/Library/CoreServices/SystemVersion.plist";
    struct stat sb;
    if ((stat(filepath, &sb) == 0) && (S_ISREG(sb.st_mode) || S_ISLNK(sb.st_mode))) {
        FILE * file = fopen(filepath, "r");

        if (file == NULL) {
            perror(filepath);
            return -1;
        }

        char line[512];

        while (fgets(line, 512, file) != NULL) {
            if (regex_matched(line, "ProductVersion")) {
                if (fgets(line, 512, file) != NULL) {
                    char * p = regex_extract(line, "[1-9][0-9.]+[0-9]");
                    size_t n = strlen(p);
                    strncpy(buf, p, bufSize > n ? n : bufSize);
                    free(p);
                    fclose(file);
                    return 0;
                }

                break;
            }
        }

        fclose(file);
    }

    return -2;
#else
    struct stat sb;

    if ((stat("/etc/os-release", &sb) == 0) && (S_ISREG(sb.st_mode) || S_ISLNK(sb.st_mode))) {
        FILE * file = fopen("/etc/os-release", "r");

        if (file == NULL) {
            perror("/etc/os-release");
            return -1;
        }

        char line[50];

        while (fgets(line, 50, file) != NULL) {
            if (regex_matched(line, "^VERSION_ID=.*")) {
                char * p = &line[11];

                if (p[0] == '"' || p[0] == '\'') {
                    p++;
                }

                size_t n = strlen(p);

                p[n - 1] = '\0';
                n--;

                if (p[n - 1] == '"' || p[n - 1] == '\'') {
                    p[n - 1] = '\0';
                    n--;
                }

                strncpy(buf, p, bufSize > n ? n : bufSize);
                return 0;
            }
        }

        fclose(file);
    }

    strncpy(buf, "rolling", bufSize > 7 ? 7 : bufSize);

    return 0;
#endif
}

int sysinfo_libc(LIBC * out) {
    struct utsname uts;

    if (uname(&uts) < 0) {
        perror("uname() error");
        return -1;
    }

    if (strcmp(uts.sysname, "Linux") == 0) {
        size_t osArchLength = strlen(uts.machine);

        size_t  dynamicLoaderPathLength = osArchLength + 19;
        char    dynamicLoaderPath[dynamicLoaderPathLength];
        memset( dynamicLoaderPath, 0, dynamicLoaderPathLength);
        sprintf(dynamicLoaderPath, "/lib/ld-musl-%s.so.1", uts.machine);

        struct stat sb;

        if ((stat(dynamicLoaderPath, &sb) == 0) && (S_ISREG(sb.st_mode) || S_ISLNK(sb.st_mode))) {
            (*out) = LIBC_MUSL;
        } else {
            if (strcmp(uts.machine, "x86_64") == 0) {
                const char * dynamicLoaderPath = "/lib64/ld-linux-x86-64.so.2";

                struct stat sb;

                if ((stat(dynamicLoaderPath, &sb) == 0) && (S_ISREG(sb.st_mode) || S_ISLNK(sb.st_mode))) {
                    (*out) = LIBC_GLIBC;
                } else {
                    (*out) = LIBC_UNKNOWN;
                }
            } else {
                size_t  dynamicLoaderPathLength = osArchLength + 22;
                char    dynamicLoaderPath[dynamicLoaderPathLength];
                memset( dynamicLoaderPath, 0, dynamicLoaderPathLength);
                sprintf(dynamicLoaderPath, "/lib64/ld-linux-%s.so.2", uts.machine);

                struct stat sb;

                if ((stat(dynamicLoaderPath, &sb) == 0) && (S_ISREG(sb.st_mode) || S_ISLNK(sb.st_mode))) {
                    (*out) = LIBC_GLIBC;
                } else {
                    (*out) = LIBC_UNKNOWN;
                }
            }
        }
    }

    return 0;
}

int sysinfo_ncpu(size_t * out) {
    long nprocs = 1;

#if defined (_SC_NPROCESSORS_ONLN)
    nprocs = sysconf(_SC_NPROCESSORS_ONLN);

    if (nprocs > 0) {
        (*out) = nprocs;
        return 0;
    }
#endif

#if defined (_SC_NPROCESSORS_CONF)
    nprocs = sysconf(_SC_NPROCESSORS_CONF);

    if (nprocs > 0) {
        (*out) = nprocs;
        return 0;
    }
#endif

    (*out) = 1;
    return 0;
}

int sysinfo_make(SysInfo * sysinfo) {
    if (sysinfo == NULL) {
        return -1;
    }

    int resultCode;

    ///////////////////////////////////////

    char osArch[31] = {0};

    resultCode = sysinfo_arch(osArch, 30);

    if (resultCode != 0) {
        return resultCode;
    }

    ///////////////////////////////////////

    char osKind[31] = {0};

    resultCode = sysinfo_kind(osKind, 30);

    if (resultCode != 0) {
        return resultCode;
    }

    ///////////////////////////////////////

    char osType[31] = {0};

    resultCode = sysinfo_type(osType, 30);

    if (resultCode != 0) {
        return resultCode;
    }

    ///////////////////////////////////////

    char osName[31] = {0};

    resultCode = sysinfo_name(osName, 30);

    if (resultCode != 0) {
        return resultCode;
    }

    ///////////////////////////////////////

    char osVers[31] = {0};

    resultCode = sysinfo_vers(osVers, 30);

    if (resultCode != 0) {
        return resultCode;
    }

    ///////////////////////////////////////

    LIBC libc = LIBC_UNKNOWN;

    resultCode = sysinfo_libc(&libc);

    if (resultCode != 0) {
        return resultCode;
    }

    ///////////////////////////////////////

    size_t ncpu = 0;
    
    resultCode = sysinfo_ncpu(&ncpu);

    if (resultCode != 0) {
        return resultCode;
    }

    ///////////////////////////////////////

    sysinfo->arch = strdup(osArch);
    sysinfo->kind = strdup(osKind);
    sysinfo->type = strdup(osType);
    sysinfo->name = strdup(osName);
    sysinfo->vers = strdup(osVers);
    sysinfo->libc = libc;
    sysinfo->ncpu = ncpu;

    return 0;
}

void sysinfo_dump(SysInfo sysinfo) {
    printf("sysinfo.ncpu: %lu\n", sysinfo.ncpu);
    printf("sysinfo.arch: %s\n",  sysinfo.arch == NULL ? "" : sysinfo.arch);
    printf("sysinfo.kind: %s\n",  sysinfo.kind == NULL ? "" : sysinfo.kind);
    printf("sysinfo.type: %s\n",  sysinfo.type == NULL ? "" : sysinfo.type);
    printf("sysinfo.name: %s\n",  sysinfo.name == NULL ? "" : sysinfo.name);
    printf("sysinfo.vers: %s\n",  sysinfo.vers == NULL ? "" : sysinfo.vers);

    switch(sysinfo.libc) {
        case LIBC_GLIBC: printf("sysinfo.libc: glibc\n"); break;
        case LIBC_MUSL:  printf("sysinfo.libc: musl\n");  break;
        default:         printf("sysinfo.libc: \n");
    }
}

void sysinfo_free(SysInfo sysinfo) {
    if (sysinfo.arch != NULL) {
        free(sysinfo.arch);
        sysinfo.arch = NULL;
    }

    if (sysinfo.kind != NULL) {
        free(sysinfo.kind);
        sysinfo.kind = NULL;
    }

    if (sysinfo.type != NULL) {
        free(sysinfo.type);
        sysinfo.type = NULL;
    }

    if (sysinfo.name != NULL) {
        free(sysinfo.name);
        sysinfo.name = NULL;
    }

    if (sysinfo.vers != NULL) {
        free(sysinfo.vers);
        sysinfo.vers = NULL;
    }
}
