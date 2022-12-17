#include "core/base64.h"
#include "ppkg.h"
#include <string.h>

int ppkg_util_base64_encode_of_string(const char * str) {
    char * result = NULL;

    int resultCode = base64_encode_of_string(&result, str);

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
    }

    if (result != NULL) {
        free(result);
    }

    return resultCode;
}
