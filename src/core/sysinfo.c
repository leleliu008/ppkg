#include "sysinfo.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include "regex/regex.h"

#include <unistd.h>

int sysinfo_kind(char * * out) {
#if defined (_WIN32)
    (*out) = strdup("windows");
    return 0;
#endif

#if defined (__APPLE__)
    (*out) = strdup("darwin");
    return 0;
#endif

#if defined (__FreeBSD__)
    (*out) = strdup("freebsd");
    return 0;
#endif

#if defined (__OpenBSD__)
    (*out) = strdup("openbsd");
    return 0;
#endif

#if defined (__NetBSD__)
    (*out) = strdup("netbsd");
    return 0;
#endif

#if defined (__ANDROID__)
    (*out) = strdup("android");
    return 0;
#endif

#if defined (__linux__)
    (*out) = strdup("linux");
    return 0;
#endif

    struct utsname uts;

    if (uname(&uts) < 0) {
        perror("uname() error");
        return -1;
    }

    char * kind = strdup(uts.sysname);

    size_t kindLength = strlen(kind);

    for (size_t i = 0; i < kindLength; i++) {
        if ((kind[i] >= 'A') && (kind[i] <= 'Z')) {
             kind[i] += 32;
        }
    }

    (*out) = kind;

    return 0;
}

int sysinfo_type(char * * out) {
#if defined (_WIN32)
    (*out) = strdup("windows");
    return 0;
#endif

#if defined (__APPLE__)
    (*out) = strdup("macos");
    return 0;
#endif

#if defined (__FreeBSD__)
    (*out) = strdup("freebsd");
    return 0;
#endif

#if defined (__OpenBSD__)
    (*out) = strdup("openbsd");
    return 0;
#endif

#if defined (__NetBSD__)
    (*out) = strdup("netbsd");
    return 0;
#endif

#if defined (__ANDROID__)
    (*out) = strdup("android");
    return 0;
#endif

#if defined (__linux__)
    (*out) = strdup("linux");
    return 0;
#endif

    struct utsname uts;

    if (uname(&uts) < 0) {
        perror("uname() error");
        return -1;
    }

    char * kind = strdup(uts.sysname);

    if ((kind[0] >= 'A') && (kind[0] <= 'Z')) {
         kind[0] += 32;
    }

    (*out) = kind;

    return 0;
}

int sysinfo_arch(char * * out) {
    struct utsname uts;

    if (uname(&uts) < 0) {
        perror("uname() error");
        return -1;
    }

    (*out) = strdup(uts.machine);

    return 0;
}

int sysinfo_name(char * * out) {
#if defined (_WIN32)
    (*out) = strdup("windows");
    return 0;
#endif

#if defined (__APPLE__)
    (*out) = strdup("macos");
    return 0;
#endif

#if defined (__FreeBSD__)
    (*out) = strdup("freebsd");
    return 0;
#endif

#if defined (__OpenBSD__)
    (*out) = strdup("openbsd");
    return 0;
#endif

#if defined (__NetBSD__)
    (*out) = strdup("netbsd");
    return 0;
#endif

#if defined (__ANDROID__)
    (*out) = strdup("android");
    return 0;
#endif

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
                size_t n = strlen(line);

                line[n - 1] = '\0';

                if (line[n - 2] == '"') {
                    line[n - 2] = '\0';
                }

                if (line[3] == '"') {
                    (*out) = strdup(&line[4]);
                    fclose(file);
                    return 0;
                } else {
                    (*out) = strdup(&line[3]);
                    fclose(file);
                    return 0;
                }

            }
        }

        fclose(file);
    }

    (*out) = strdup("unknown");

    return 0;
}

int sysinfo_vers(char * * out) {
#if defined (__NetBSD__) || defined (__OpenBSD__)
    struct utsname uts;

    if (uname(&uts) < 0) {
        perror("uname() error");
        return -1;
    }

    (*out) = strdup(uts.release);

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
                    (*out) = regex_extract(line, "[1-9][0-9.]+[0-9]");
                    fclose(file);
                    return 0;
                }

                break;
            }
        }

        fclose(file);
    }

    (*out) = NULL;

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
            if (regex_matched(line, "^VERSION_ID=.*")) {
                size_t n = strlen(line);

                line[n - 1] = '\0';

                if (line[n - 2] == '"') {
                    line[n - 2] = '\0';
                }

                if (line[11] == '"') {
                    (*out) = strdup(&line[12]);
                    fclose(file);
                    return 0;
                } else {
                    (*out) = strdup(&line[11]);
                    fclose(file);
                    return 0;
                }
            }
        }

        fclose(file);
    }

    (*out) = strdup("rolling");

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
        char dynamicLoaderPath[40] = {0};
        sprintf(dynamicLoaderPath, "/lib/ld-musl-%s.so.1", uts.machine);

        struct stat sb;

        if ((stat(dynamicLoaderPath, &sb) == 0) && (S_ISREG(sb.st_mode) || S_ISLNK(sb.st_mode))) {
            (*out) = LIBC_MUSL;
        } else {
            memset(dynamicLoaderPath, 0, 40);

            if (strcmp(uts.machine, "x86_64") == 0) {
                strcpy(dynamicLoaderPath, "/lib64/ld-linux-x86-64.so.2");
            } else {
                sprintf(dynamicLoaderPath, "/lib64/ld-linux-%s.so.2", uts.machine);
            }

            if ((stat(dynamicLoaderPath, &sb) == 0) && (S_ISREG(sb.st_mode) || S_ISLNK(sb.st_mode))) {
                (*out) = LIBC_GLIBC;
            } else {
                (*out) = LIBC_UNKNOWN;
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

int sysinfo_make(SysInfo * * out) {
    SysInfo * sysinfo = (SysInfo*)calloc(1, sizeof(SysInfo));

    char * arch = NULL;
    sysinfo_arch(&arch);
    sysinfo->arch = arch;

    char * kind = NULL;
    sysinfo_kind(&kind);
    sysinfo->kind = kind;

    char * type = NULL;
    sysinfo_type(&type);
    sysinfo->type = type;

    char * name = NULL;
    sysinfo_name(&name);
    sysinfo->name = name;

    char * vers = NULL;
    sysinfo_vers(&vers);
    sysinfo->vers = vers;

    LIBC libc = LIBC_UNKNOWN;
    sysinfo_libc(&libc);
    sysinfo->libc = libc;

    size_t ncpu = 0;
    sysinfo_ncpu(&ncpu);
    sysinfo->ncpu = ncpu;

    (*out) = sysinfo;

    return 0;
}

void sysinfo_dump(SysInfo * sysinfo) {
    if (sysinfo == NULL) {
        return;
    }

    printf("sysinfo.ncpu: %lu\n", sysinfo->ncpu);
    printf("sysinfo.arch: %s\n",  sysinfo->arch == NULL ? "" : sysinfo->arch);
    printf("sysinfo.kind: %s\n",  sysinfo->kind == NULL ? "" : sysinfo->kind);
    printf("sysinfo.type: %s\n",  sysinfo->type == NULL ? "" : sysinfo->type);
    printf("sysinfo.name: %s\n",  sysinfo->name == NULL ? "" : sysinfo->name);
    printf("sysinfo.vers: %s\n",  sysinfo->vers == NULL ? "" : sysinfo->vers);

    switch(sysinfo->libc) {
        case LIBC_GLIBC: printf("sysinfo.libc: glibc\n"); break;
        case LIBC_MUSL:  printf("sysinfo.libc: musl\n");  break;
        default:         printf("sysinfo.libc: unknown\n");
    }
}

void sysinfo_free(SysInfo * sysinfo) {
    if (sysinfo == NULL) {
        return;
    }

    if (sysinfo->arch != NULL) {
        free(sysinfo->arch);
        sysinfo->arch = NULL;
    }

    if (sysinfo->kind != NULL) {
        free(sysinfo->kind);
        sysinfo->kind = NULL;
    }

    if (sysinfo->type != NULL) {
        free(sysinfo->type);
        sysinfo->type = NULL;
    }

    if (sysinfo->name != NULL) {
        free(sysinfo->name);
        sysinfo->name = NULL;
    }

    if (sysinfo->vers != NULL) {
        free(sysinfo->vers);
        sysinfo->vers = NULL;
    }

    free(sysinfo);
}
