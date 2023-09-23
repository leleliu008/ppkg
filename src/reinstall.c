#include "ppkg.h"

int ppkg_reinstall(const char * packageName, PPKGInstallOptions options) {
    options.force = true;
    return ppkg_install(packageName, options);
}
