#include "core/log.h"

#include "ppkg.h"

int ppkg_cleanup(const bool verbose) {
    if (verbose) {
        LOG_SUCCESS1("Done.");
    }

    return PPKG_OK;
}
