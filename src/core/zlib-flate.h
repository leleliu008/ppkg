#ifndef PPKG_ZLIB_FLATE_H
#define PPKG_ZLIB_FLATE_H

int zlib_deflate_string_to_file(const char * inputBuf, size_t inputBufSizeInBytes, FILE * file);
int zlib_inflate_file_to_file(FILE * inputFile, FILE * outputFile);

#endif
