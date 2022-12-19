#include "ppkg.h"

int ppkg_upgrade(const char * packageName, PPKGInstallOptions options) {
    int resultCode = ppkg_check_if_the_given_package_is_outdated(packageName);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    resultCode = ppkg_uninstall(packageName, options.verbose);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    return ppkg_install(packageName, options);
}
