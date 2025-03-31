#include <stdio.h>

#include <limits.h>
#include <sys/syslimits.h>

#include "core/self.h"

#include "ppkg.h"

int ppkg_about(const bool verbose) {
    char buf[PATH_MAX];

    int ret = ppkg_home_dir(buf, NULL);

    if (ret != PPKG_OK) {
        return ret;
    }

    printf("ppkg.version : %s\n", PPKG_VERSION);
    printf("ppkg.homedir : %s\n", buf);

    ret = selfpath(buf);

    if (ret == -1) {
        perror(NULL);
        return PPKG_ERROR;
    }

    printf("ppkg.exepath : %s\n", buf);

    printf("ppkg.website : %s\n\n", "https://github.com/leleliu008/ppkg");
   
    return ppkg_buildinfo();
}
