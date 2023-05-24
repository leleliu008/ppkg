#include "ppkg.h"

int ppkg_reinstall(const char * packageName, PPKGInstallOptions options) {
    int ret = ppkg_check_if_the_given_package_is_available(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    ret = ppkg_check_if_the_given_package_is_installed(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    ret = ppkg_uninstall(packageName, options.logLevel >= PPKGLogLevel_verbose);

    if (ret != PPKG_OK) {
        return ret;
    }

    return ppkg_install(packageName, options);
}
