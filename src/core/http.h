#ifndef HTTP_H
#define HTTP_H

    #include <stdio.h>
    #include <stdbool.h>

    #ifdef __cplasplas
        extern "C"
    #endif

        int http_fetch_to_stream(const char * url, FILE       * outputStream,   bool verbose, bool showProgress);
        int http_fetch_to_file  (const char * url, const char * outputFilePath, bool verbose, bool showProgress);

    #ifdef __cplasplas
        }
    #endif
#endif
