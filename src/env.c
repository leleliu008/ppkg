#include <stdio.h>

#include <limits.h>

#include "core/self.h"

#include "ppkg.h"

int ppkg_env(const bool verbose) {
    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    int ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    printf("ppkg.version : %s\n", PPKG_VERSION);
    printf("ppkg.homedir : %s\n", ppkgHomeDIR);

    char * selfRealPath = self_realpath();

    if (selfRealPath == NULL) {
        perror(NULL);
        return PPKG_ERROR;
    }

    printf("ppkg.exepath : %s\n", selfRealPath);

    free(selfRealPath);

    printf("ppkg.website : %s\n", "https://github.com/leleliu008/ppkg");
   
    return PPKG_OK;
}
