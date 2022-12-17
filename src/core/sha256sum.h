#ifndef SHA256SUM_H
#define SHA256SUM_H
    
    #include <stdio.h>
    #include <stdlib.h>
     
    #ifdef __cplusplus
        extern "C" {
    #endif
            char* sha256sum_of_bytes (unsigned char * inputBuffer, size_t inputBufferSizeInBytes);
            char* sha256sum_of_string(const char * str);
            char* sha256sum_of_file(const char * filepath);
            char* sha256sum_of_stream(FILE * file);
    #ifdef __cplusplus
        }
    #endif
#endif
