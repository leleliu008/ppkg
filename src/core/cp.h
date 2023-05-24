#ifndef _CP_H
#define _CP_H

/** copy a file from one place to other place.
 *
 *  On success, 0 is returned.
 *  On error,  -1 is returned and errno is set to indicate the error.
 */
int copy_file(const char * fromFilePath, const char * toFilePath);

#endif
