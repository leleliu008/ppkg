#include "ppkg.h"

int ppkg_formula_view(const char * packageName, bool raw) {
    if (raw) {
        return ppkg_formula_cat(packageName);
    } else {
        return ppkg_formula_bat(packageName);
    }
}
