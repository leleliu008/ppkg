#include <string.h>

#include "ppkg.h"

int ppkg_reinstall(const char * packageName, const PPKGTargetPlatform * targetPlatform, const PPKGInstallOptions * installOptions) {
    PPKGInstallOptions installOptionsCopy;

    memcpy(&installOptionsCopy, installOptions, sizeof(PPKGInstallOptions));

    installOptionsCopy.force = true;

    return ppkg_install(packageName, targetPlatform, &installOptionsCopy);
}
