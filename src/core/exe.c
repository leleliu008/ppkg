#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "exe.h"

int exe_search(const char * commandName, char *** listP, size_t * listSize, bool findAll) {
    if (commandName == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    size_t commandNameLength = strlen(commandName);

    if (commandNameLength == 0) {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    if (listP == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (listSize == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    char * PATH = getenv("PATH");

    if (PATH == NULL) {
        return PPKG_ERROR_ENV_PATH_NOT_SET;
    }

    size_t PATHLength = strlen(PATH);

    if (PATHLength == 0) {
        return PPKG_ERROR_ENV_PATH_NOT_SET;
    }

    size_t  PATH2Length = PATHLength + 1;
    char    PATH2[PATH2Length];
    strncpy(PATH2, PATH, PATH2Length);

    struct stat st;

    char ** stringArrayList = NULL;
    size_t  stringArrayListCapcity = 0;
    size_t  stringArrayListSize    = 0;

    char * PATHItem = strtok(PATH2, ":");

    while (PATHItem != NULL) {
        if ((stat(PATHItem, &st) == 0) && S_ISDIR(st.st_mode)) {
            size_t fullPathLength = strlen(PATHItem) + commandNameLength + 2;
            char   fullPath[fullPathLength];
            snprintf(fullPath, fullPathLength, "%s/%s", PATHItem, commandName);

            if (access(fullPath, X_OK) == 0) {
                if (stringArrayListCapcity == stringArrayListSize) {
                    stringArrayListCapcity += 2;

                    char** paths = (char**)realloc(stringArrayList, stringArrayListCapcity * sizeof(char*));

                    if (paths == NULL) {
                        if (stringArrayList != NULL) {
                            for (size_t i = 0; i < stringArrayListSize; i++) {
                                free(stringArrayList[i]);
                                stringArrayList[i] = NULL;
                            }
                            free(stringArrayList);
                        }
                        return PPKG_ERROR_MEMORY_ALLOCATE;
                    } else {
                        stringArrayList = paths;
                    }
                }

                char * fullPathDup = strdup(fullPath);

                if (fullPathDup == NULL) {
                    if (stringArrayList != NULL) {
                        for (size_t i = 0; i < stringArrayListSize; i++) {
                            free(stringArrayList[i]);
                            stringArrayList[i] = NULL;
                        }
                        free(stringArrayList);
                    }
                    return PPKG_ERROR_MEMORY_ALLOCATE;
                }

                stringArrayList[stringArrayListSize] = fullPathDup;
                stringArrayListSize += 1;

                if (!findAll) {
                    break;
                }
            }
        }

        PATHItem = strtok(NULL, ":");
    }

    (*listP)    = stringArrayList;
    (*listSize) = stringArrayListSize;

    return stringArrayList == NULL ? PPKG_ERROR_EXE_NOT_FOUND : PPKG_OK;
}

int exe_lookup(const char * commandName, char ** pathP, size_t * pathLength) {
    if (commandName == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    size_t commandNameLength = strlen(commandName);

    if (commandNameLength == 0) {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    if (pathP == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    char * PATH = getenv("PATH");

    if (PATH == NULL) {
        return PPKG_ERROR_ENV_PATH_NOT_SET;
    }

    size_t PATHLength = strlen(PATH);

    if (PATHLength == 0) {
        return PPKG_ERROR_ENV_PATH_NOT_SET;
    }

    size_t  PATH2Length = PATHLength + 1;
    char    PATH2[PATH2Length];
    strncpy(PATH2, PATH, PATH2Length);

    struct stat st;

    char * PATHItem = strtok(PATH2, ":");

    while (PATHItem != NULL) {
        if ((stat(PATHItem, &st) == 0) && S_ISDIR(st.st_mode)) {
            size_t fullPathLength = strlen(PATHItem) + commandNameLength + 2;
            char   fullPath[fullPathLength];
            snprintf(fullPath, fullPathLength, "%s/%s", PATHItem, commandName);

            if (access(fullPath, X_OK) == 0) {
                char * fullPathDup = strdup(fullPath);

                if (fullPathDup == NULL) {
                    return PPKG_ERROR_MEMORY_ALLOCATE;
                }

                (*pathP) = fullPathDup;

                if (pathLength != NULL) {
                    (*pathLength) = fullPathLength;
                }

                return PPKG_OK;
            }
        }

        PATHItem = strtok(NULL, ":");
    }

    return PPKG_ERROR_EXE_NOT_FOUND;
}

int exe_lookup2(const char * commandName, char buf[], size_t * writtenSize, size_t maxSize) {
    if (commandName == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    size_t commandNameLength = strlen(commandName);

    if (commandNameLength == 0) {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    if (buf == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (maxSize == 0) {
        return PPKG_ERROR_ARG_IS_INVALID;
    }

    char * PATH = getenv("PATH");

    if (PATH == NULL) {
        return PPKG_ERROR_ENV_PATH_NOT_SET;
    }

    size_t PATHLength = strlen(PATH);

    if (PATHLength == 0) {
        return PPKG_ERROR_ENV_PATH_NOT_SET;
    }

    size_t  PATH2Length = PATHLength + 1;
    char    PATH2[PATH2Length];
    strncpy(PATH2, PATH, PATH2Length);

    struct stat st;

    char * PATHItem = strtok(PATH2, ":");

    while (PATHItem != NULL) {
        if ((stat(PATHItem, &st) == 0) && S_ISDIR(st.st_mode)) {
            size_t fullPathLength = strlen(PATHItem) + commandNameLength + 2;
            char   fullPath[fullPathLength];
            snprintf(fullPath, fullPathLength, "%s/%s", PATHItem, commandName);

            if (access(fullPath, X_OK) == 0) {
                size_t n = maxSize > fullPathLength ? fullPathLength : maxSize;

                strncpy(buf, fullPath, n);

                if (writtenSize != NULL) {
                    (*writtenSize) = n;
                }

                return PPKG_OK;
            }
        }

        PATHItem = strtok(NULL, ":");
    }

    return PPKG_ERROR_EXE_NOT_FOUND;
}
