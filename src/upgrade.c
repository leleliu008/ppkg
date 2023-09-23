#include "ppkg.h"

int ppkg_upgrade(const char * packageName, PPKGInstallOptions options) {
    int ret = ppkg_check_if_the_given_package_is_outdated(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    options.force = true;

    return ppkg_install(packageName, options);
}
