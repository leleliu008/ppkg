#ifndef PPKG_EXE_H
#define PPKG_EXE_H

#include <stdlib.h>
#include <stdbool.h>

#include "../ppkg.h"

int exe_search(const char * commandName, char *** listP, size_t * listSize, bool findAll);
int exe_lookup(const char * commandName, char **  pathP, size_t * pathLength);
int exe_lookup2(const char * commandName, char buf[], size_t * writtenSize, size_t maxSize);
int exe_self_realpath(char ** pathP);

#endif
