/* 
 * https://www.gnu.org/software/gnulib/manual/html_node/environ.html
 *
 * https://pubs.opengroup.org/onlinepubs/9799919799/functions/environ.html
 *
 * http://refspecs.linux-foundation.org/LSB_4.0.0/LSB-Core-generic/LSB-Core-generic/baselib---environ.html
 */

#include <stdio.h>

#ifdef __APPLE__
#include <crt_externs.h>
#else
#include <unistd.h>
#endif

void printenv() {
#if defined (__APPLE__)
    char **envlist = *_NSGetEnviron();
#elif defined (__linux__)
    char **envlist = __environ;
#else
    extern char ** environ;
    char **envlist = environ;
#endif

    for (int i = 0; envlist[i] != NULL; i++) {
        puts(envlist[i]);
    }
}
