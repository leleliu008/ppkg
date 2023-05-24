#include "base64.h"

#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>

int base64_encode_of_string(char * * output, size_t * outputSizeInBytes, const char * input, size_t inputSizeInBytes) {
    if (output == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (input == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    return base64_encode_of_bytes(output, outputSizeInBytes, (unsigned char *)input, inputSizeInBytes == 0 ? strlen(input) : inputSizeInBytes);
}

int base64_encode_of_bytes(char * * output, size_t * outputSizeInBytes, const unsigned char * input, size_t inputSizeInBytes) {
    if (output == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (input == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (inputSizeInBytes == 0) {
        return PPKG_ERROR_ARG_IS_INVALID;
    }

    size_t bufLength = (inputSizeInBytes << 2) / 3 + 3;
    char   buf[bufLength];
    memset(buf, 0, bufLength);

    int n = EVP_EncodeBlock((unsigned char *)buf, input, inputSizeInBytes);

    if (outputSizeInBytes != NULL) {
        (*outputSizeInBytes) = n;
    }

    (*output) = strndup(buf, n);

    return (*output) == NULL ? PPKG_ERROR_MEMORY_ALLOCATE : PPKG_OK;
}

int base64_decode_to_bytes(unsigned char * * output, size_t * outputSizeInBytes, const char * input, size_t inputSizeInBytes) {
    if (output == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (input == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (inputSizeInBytes == 0) {
        inputSizeInBytes = strlen(input);
    }

    if (inputSizeInBytes == 0) {
        return PPKG_ERROR_ARG_IS_INVALID;
    }

    size_t bufLength = (inputSizeInBytes * 3) >> 2;
    unsigned char  buf[bufLength];
    memset(buf, 0, bufLength);

    int n = EVP_DecodeBlock(buf, (unsigned char *)input, inputSizeInBytes);

    if (n == -1) {
        return PPKG_ERROR_ARG_IS_INVALID;
    }

    unsigned char * result = (unsigned char *)calloc(n, sizeof(unsigned char));

    if (result == NULL) {
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    memcpy(result, buf, n);

    (*output) = result;

    if (outputSizeInBytes != NULL) {
        (*outputSizeInBytes) = n;
    }

    return PPKG_OK;
}

int base64_decode_to_string(char * * output, size_t * outputSizeInBytes, const char * input, size_t inputSizeInBytes) {
    if (output == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (input == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (inputSizeInBytes == 0) {
        inputSizeInBytes = strlen(input);
    }

    if (inputSizeInBytes == 0) {
        return PPKG_ERROR_ARG_IS_INVALID;
    }

    size_t bufLength = ((inputSizeInBytes * 3) >> 2) + 1;
    char   buf[bufLength];
    memset(buf, 0, bufLength);

    int n = EVP_DecodeBlock((unsigned char *)buf, (unsigned char *)input, inputSizeInBytes);

    if (n == -1) {
        return PPKG_ERROR_ARG_IS_INVALID;
    }

    if (outputSizeInBytes != NULL) {
        (*outputSizeInBytes) = n;
    }

    (*output) = strndup(buf, n);

    return (*output) == NULL ? PPKG_ERROR_MEMORY_ALLOCATE : PPKG_OK;
}
