#include "ppkg.h"
#include "core/log.h"

int ppkg_cleanup(bool verbose) {
    if (verbose) {
        LOG_SUCCESS1("Done.");
    }

    return PPKG_OK;
}
