#include <stdio.h>
#include <errno.h>

#include "cp.h"

int copy_file(const char * fromFilePath, const char * toFilePath) {
    FILE * fromFile = fopen(fromFilePath, "rb");

    if (fromFile == NULL) {
        return -1;
    }

    FILE * toFile = fopen(toFilePath, "wb");

    if (toFile == NULL) {
        int err = errno;
        fclose(fromFile);
        errno = err;
        return -1;
    }

    unsigned char buff[1024];

    for (;;) {
        size_t size = fread(buff, 1, 1024, fromFile);

        if (ferror(fromFile)) {
            fclose(fromFile);
            fclose(toFile);
            errno = EIO;
            return -1;
        }

        if (size > 0U) {
            if ((fwrite(buff, 1, size, toFile) != size) || ferror(toFile)) {
                fclose(fromFile);
                fclose(toFile);
                errno = EIO;
                return -1;
            }
        }

        if (feof(fromFile)) {
            fclose(fromFile);
            fclose(toFile);
            return 0;
        }
    }
}
