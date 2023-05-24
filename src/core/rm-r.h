#ifndef _CORE_RM_R_H
#define _CORE_RM_R_H

#include<stdbool.h>

/**
 * remove the given directory and it's contents recursively
 *
 *  On success, 0 is returned.
 *  On error,  -1 is returned and errno is set to indicate the error.
 */
int rm_r(const char * dirPath, bool verbose);

#endif
