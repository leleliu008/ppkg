#ifndef BASE64_H
#define BASE64_H

#include <stdlib.h>

int base64_encode_of_bytes( char * * out, const unsigned char * input, size_t nBytes);
int base64_encode_of_string(char * * out, const char * input);

int base64_decode_to_bytes(unsigned char * * out, size_t * writtenNBytes, const char * input);
int base64_decode_to_string(        char * * out, size_t * writtenNBytes, const char * input);

#endif
