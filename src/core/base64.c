#include "base64.h"

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>

int base64_encode_of_string(char * * out, const char * input) {
    if (input == NULL) {
        return -1;
    }

    return base64_encode_of_bytes(out, (unsigned char *)input, strlen(input));
}

int base64_encode_of_bytes(char * * out, const unsigned char * input, size_t nBytes) {
    if (input == NULL) {
        return -1;
    }

    if (nBytes == 0) {
        return -2;
    }

    size_t bufLength = nBytes * 8 / 6 + 3;
    char   buf[bufLength];
    memset(buf, 0, bufLength);

    int n = EVP_EncodeBlock((unsigned char *)buf, input, nBytes);

    (*out) = strndup(buf, n);

    return 0;
}

int base64_decode_to_bytes(unsigned char * * out, size_t * writtenNBytes, const char * input) {
    if (input == NULL) {
        return -1;
    }

    size_t nBytes = strlen(input);

    if (nBytes == 0) {
        return -2;
    }

    size_t bufLength = nBytes * 6 / 8;
    unsigned char  buf[bufLength];
    memset(buf, 0, bufLength);

    int n = EVP_DecodeBlock(buf, (unsigned char *)input, nBytes);

    if (n == -1) {
        return -3;
    }

    unsigned char * result = (unsigned char *)calloc(n, sizeof(unsigned char));

    memcpy(result, buf, n);

    (*out) = result;
    (*writtenNBytes) = n;

    return 0;
}

int base64_decode_to_string(char * * out, size_t * writtenNBytes, const char * input) {
    if (input == NULL) {
        return -1;
    }

    size_t nBytes = strlen(input);

    if (nBytes == 0) {
        return -2;
    }

    size_t bufLength = nBytes * 6 / 8 + 1;
    char   buf[bufLength];
    memset(buf, 0, bufLength);

    int n = EVP_DecodeBlock((unsigned char *)buf, (unsigned char *)input, nBytes);

    if (n == -1) {
        return -3;
    }

    (*out) = strndup(buf, n);
    (*writtenNBytes) = n;

    return 0;
}
