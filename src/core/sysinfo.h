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

int  sysinfo_kind(char * * out);
int  sysinfo_type(char * * out);
int  sysinfo_name(char * * out);
int  sysinfo_vers(char * * out);
int  sysinfo_arch(char * * out);
int  sysinfo_libc(LIBC   * out);
int  sysinfo_ncpu(size_t * out);

int  sysinfo_make(SysInfo * * sysinfo);
void sysinfo_dump(SysInfo *   sysinfo);
void sysinfo_free(SysInfo *   sysinfo);

#endif
