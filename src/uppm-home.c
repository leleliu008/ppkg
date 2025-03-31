#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "uppm.h"
#include "ppkg.h"

int uppm_home_dir(char buf[], size_t * len) {
    if (buf == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    char   tmpBuf[PATH_MAX];
    size_t tmpBufLength;

    int ret = ppkg_home_dir(tmpBuf, &tmpBufLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    const char * const str = "/uppm";
    size_t strLength = strlen(str);

    strncpy(tmpBuf + tmpBufLength, str, strLength + sizeof(char));

    struct stat st;

    if (stat(buf, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "%s was expected to be a directory, but it was not.\n", buf);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(buf, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(buf);
                return PPKG_ERROR;
            }
        }
    }

    tmpBufLength += strLength;

    if (len != NULL) {
        (*len) = tmpBufLength;
    }

    return PPKG_OK;
}
