#include <limits.h>

#include "ppkg.h"

int ppkg_formula_load(const char * packageName, const char * targetPlatformName, PPKGFormula * * out) {
    char formulaFilePath[PATH_MAX];

    int ret = ppkg_formula_path(packageName, targetPlatformName, formulaFilePath);

    if (ret != PPKG_OK) {
        return ret;
    }

    ret = ppkg_formula_parse(formulaFilePath, out);

    return ret;
}
