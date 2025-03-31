#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>

#include "ppkg.h"

int ppkg_formula_bat(const char * packageName, const char * targetPlatformName) {
    char formulaFilePath[PATH_MAX];

    int ret = ppkg_formula_path(packageName, targetPlatformName, formulaFilePath);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    ret = ppkg_home_dir(ppkgHomeDIR, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * const str = "/uppm/installed/bat/bin/bat";

    size_t batCommandPathCapacity = ppkgHomeDIRLength + strlen(str) + sizeof(char);
    char   batCommandPath[batCommandPathCapacity];

    ret = snprintf(batCommandPath, batCommandPathCapacity, "%s%s", ppkgHomeDIR, str);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    execlp(batCommandPath, batCommandPath, "--paging=never", formulaFilePath, NULL);
    perror(batCommandPath);

    return PPKG_ERROR;
}
