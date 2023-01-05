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

int base64_encode_of_bytes(char * * out, const unsigned char * input, size_t inputSizeInBytes) {
    if (input == NULL) {
        return -1;
    }

    if (inputSizeInBytes == 0) {
        return -2;
    }

    size_t bufLength = inputSizeInBytes * 8 / 6 + 3;
    char   buf[bufLength];
    memset(buf, 0, bufLength);

    int n = EVP_EncodeBlock((unsigned char *)buf, input, inputSizeInBytes);

    (*out) = strndup(buf, n);

    return 0;
}

int base64_decode_to_bytes(unsigned char * * output, size_t * outputSizeInBytes, const char * input) {
    if (input == NULL) {
        return -1;
    }

    size_t inputSizeInBytes = strlen(input);

    if (inputSizeInBytes == 0) {
        return -2;
    }

    size_t bufLength = inputSizeInBytes * 6 / 8;
    unsigned char  buf[bufLength];
    memset(buf, 0, bufLength);

    int n = EVP_DecodeBlock(buf, (unsigned char *)input, inputSizeInBytes);

    if (n == -1) {
        return -3;
    }

    unsigned char * result = (unsigned char *)calloc(n, sizeof(unsigned char));

    memcpy(result, buf, n);

    (*output) = result;
    (*outputSizeInBytes) = n;

    return 0;
}

int base64_decode_to_string(char * * output, size_t * outputSizeInBytes, const char * input) {
    if (input == NULL) {
        return -1;
    }

    size_t inputSizeInBytes = strlen(input);

    if (inputSizeInBytes == 0) {
        return -2;
    }

    size_t bufLength = inputSizeInBytes * 6 / 8 + 1;
    char   buf[bufLength];
    memset(buf, 0, bufLength);

    int n = EVP_DecodeBlock((unsigned char *)buf, (unsigned char *)input, inputSizeInBytes);

    if (n == -1) {
        return -3;
    }

    (*output) = strndup(buf, n);
    (*outputSizeInBytes) = n;

    return 0;
}
