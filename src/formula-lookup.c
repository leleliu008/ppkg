#include "ppkg.h"

int ppkg_formula_lookup(const char * packageName, PPKGFormula * * out) {
    char * formulaFilePath = NULL;

    int ret = ppkg_formula_locate(packageName, &formulaFilePath);

    if (ret != PPKG_OK) {
        return ret;
    }

    ret = ppkg_formula_parse(formulaFilePath, out);

    free(formulaFilePath);

    return ret;
}
