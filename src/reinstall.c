#include "ppkg.h"

int ppkg_reinstall(const char * packageName, bool verbose) {
    int resultCode = ppkg_check_if_the_given_package_is_available(packageName);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    resultCode = ppkg_check_if_the_given_package_is_installed(packageName);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    resultCode = ppkg_uninstall(packageName, verbose);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    return ppkg_install(packageName, verbose);
}
