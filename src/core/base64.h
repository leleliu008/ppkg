#ifndef BASE64_H
#define BASE64_H

#include <stdlib.h>

int base64_encode_of_bytes( char * * output, const unsigned char * input, size_t inputSizeInBytes);
int base64_encode_of_string(char * * output, const char * input);

int base64_decode_to_bytes(unsigned char * * output, size_t * outputSizeInBytes, const char * input);
int base64_decode_to_string(        char * * output, size_t * outputSizeInBytes, const char * input);

#endif
