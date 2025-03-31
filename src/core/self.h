#ifndef _SELF_H
#define _SELF_H

/** get the current running executable's absolute path.
 *
 *  the capacity of buf must be PATH_MAX
 *
 *  On success, 0 returned.
 *
 *  On error, -1 is returned and errno is set to indicate the error.
 */
int selfpath(char buf[]);

#endif
