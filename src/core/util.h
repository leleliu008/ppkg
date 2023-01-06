#ifndef PPKG_UTIL_H
#define PPKG_UTIL_H

int get_file_extension_from_url(char * buf, size_t bufSize, const char * url);

int get_current_executable_realpath(char * * out);

#endif
