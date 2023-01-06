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
        return -1;
    }

    if (inputBuffer == NULL) {
        return -3;
    }

    if (inputBufferSizeInBytes == 0) {
        return -4;
    }

    unsigned char sha256Bytes[SHA256_DIGEST_LENGTH] = {0};
 
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, inputBuffer, inputBufferSizeInBytes);
    SHA256_Final(sha256Bytes, &ctx);

    tohex(outputBuffer, sha256Bytes);

    return 0;
}

int sha256sum_of_string(char outputBuffer[65], const char * str) {
    unsigned char sha256Bytes[SHA256_DIGEST_LENGTH] = {0};
 
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, str, strlen(str));
    SHA256_Final(sha256Bytes, &ctx);

    tohex(outputBuffer, sha256Bytes);

    return 0;
}

int sha256sum_of_stream(char outputBuffer[65], FILE * file) {
    if (outputBuffer == NULL) {
        return -1;
    }

    unsigned char sha256Bytes[SHA256_DIGEST_LENGTH] = {0};
 
    SHA256_CTX ctx;
    SHA256_Init(&ctx);

    unsigned char buffer[1024] = {0};
    size_t size = 0;

    while ((size = fread(buffer, 1, 1024, file)) != 0) {
        SHA256_Update(&ctx, buffer, size);
    }

    SHA256_Final(sha256Bytes, &ctx);

    tohex(outputBuffer, sha256Bytes);

    return 0;
}

int sha256sum_of_file(char outputBuffer[65], const char * filepath) {
    if (outputBuffer == NULL) {
        return -1;
    }

    if ((filepath == NULL) || (strcmp(filepath, "") == 0)) {
        return -3;
    }

    FILE * file = fopen(filepath, "rb");

    if (file == NULL) {
        perror(filepath);
        return -4;
    }

    int resultCode = sha256sum_of_stream(outputBuffer, file);

    fclose(file);

    return resultCode;
}
