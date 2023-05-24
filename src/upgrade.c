#include "ppkg.h"

int ppkg_upgrade(const char * packageName, PPKGInstallOptions options) {
    int ret = ppkg_check_if_the_given_package_is_outdated(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    ret = ppkg_uninstall(packageName, options.logLevel >= PPKGLogLevel_verbose);

    if (ret != PPKG_OK) {
        return ret;
    }

    return ppkg_install(packageName, options);
}
