#include "core/base64.h"
#include "core/log.h"
#include "ppkg.h"

#include <string.h>

int ppkg_util_base64_decode_to_string(const char * base64EncodedStr) {
    char * result = NULL;
    size_t n;

    int resultCode = base64_decode_to_string(&result, &n, base64EncodedStr);

    switch (resultCode) {
        case 0:
            printf("%s\n", result);
            break;
        case -1:
            resultCode = PPKG_ARG_IS_NULL;
            break;
        case -2:
            resultCode = PPKG_ARG_IS_EMPTY;
            break;
        case -3:
            resultCode = PPKG_ARG_IS_INVALID;
            break;
    }

    if (result != NULL) {
        free(result);
    }

    return resultCode;
}
