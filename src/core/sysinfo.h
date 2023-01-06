#ifndef SYSINFO_H
#define SYSINFO_H

#include <stdlib.h>

typedef enum {
    LIBC_UNKNOWN,
    LIBC_GLIBC,
    LIBC_MUSL
} LIBC;

typedef struct {
   char * kind;
   char * type;
   char * name;
   char * vers;
   char * arch;
   LIBC   libc;
   size_t ncpu;
} SysInfo;

int  sysinfo_kind(char * buf, size_t bufSize);
int  sysinfo_type(char * buf, size_t bufSize);
int  sysinfo_name(char * buf, size_t bufSize);
int  sysinfo_vers(char * buf, size_t bufSize);
int  sysinfo_arch(char * buf, size_t bufSize);
int  sysinfo_libc(LIBC * libc);
int  sysinfo_ncpu(size_t * out);

int  sysinfo_make(SysInfo * sysinfo);
void sysinfo_dump(SysInfo sysinfo);
void sysinfo_free(SysInfo sysinfo);

#endif
