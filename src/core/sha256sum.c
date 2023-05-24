#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

#include "sha256sum.h"

static inline void tohex(char buf[65], unsigned char * sha256Bytes) {
    const char * table = "0123456789abcdef";

    size_t i, j;

    for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        j = i << 1;
        buf[j]     = table[sha256Bytes[i] >> 4];
        buf[j + 1] = table[sha256Bytes[i] & 0x0F];
    }
}

int sha256sum_of_bytes(char outputBuffer[65], unsigned char * inputBuffer, size_t inputBufferSizeInBytes) {
    if (outputBuffer == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (inputBuffer == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (inputBufferSizeInBytes == 0) {
        return PPKG_ERROR_ARG_IS_INVALID;
    }

    unsigned char sha256Bytes[SHA256_DIGEST_LENGTH] = {0};
 
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, inputBuffer, inputBufferSizeInBytes);
    SHA256_Final(sha256Bytes, &ctx);

    tohex(outputBuffer, sha256Bytes);

    return PPKG_OK;
}

int sha256sum_of_string(char outputBuffer[65], const char * str) {
    if (str == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    size_t strLength = strlen(str);

    if (strLength == 0) {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    unsigned char sha256Bytes[SHA256_DIGEST_LENGTH] = {0};
 
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, str, strLength);
    SHA256_Final(sha256Bytes, &ctx);

    tohex(outputBuffer, sha256Bytes);

    return PPKG_OK;
}

int sha256sum_of_stream(char outputBuffer[65], FILE * file) {
    if (outputBuffer == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (file == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    unsigned char sha256Bytes[SHA256_DIGEST_LENGTH] = {0};
 
    SHA256_CTX ctx;
    SHA256_Init(&ctx);

    unsigned char buffer[1024];
    size_t size;

    for (;;) {
        size = fread(buffer, 1, 1024, file);

        if (ferror(file)) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (size > 0) {
            SHA256_Update(&ctx, buffer, size);
        }

        if (feof(file)) {
            break;
        }
    }

    SHA256_Final(sha256Bytes, &ctx);

    tohex(outputBuffer, sha256Bytes);

    return PPKG_OK;
}

int sha256sum_of_file(char outputBuffer[65], const char * filepath) {
    if (outputBuffer == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (filepath == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (filepath[0] == '\0') {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    FILE * file = fopen(filepath, "rb");

    if (file == NULL) {
        perror(filepath);
        return PPKG_ERROR;
    }

    int ret = sha256sum_of_stream(outputBuffer, file);

    fclose(file);

    return ret;
}
