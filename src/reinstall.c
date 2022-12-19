#include "ppkg.h"

int ppkg_reinstall(const char * packageName, PPKGInstallOptions options) {
    int resultCode = ppkg_check_if_the_given_package_is_available(packageName);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    resultCode = ppkg_check_if_the_given_package_is_installed(packageName);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    resultCode = ppkg_uninstall(packageName, options.verbose);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    return ppkg_install(packageName, options);
}
