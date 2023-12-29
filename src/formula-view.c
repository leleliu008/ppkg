#include "ppkg.h"

int ppkg_formula_view(const char * packageName, const char * targetPlatformName, const bool raw) {
    if (raw) {
        return ppkg_formula_cat(packageName, targetPlatformName);
    } else {
        return ppkg_formula_bat(packageName, targetPlatformName);
    }
}
