#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "exe.h"

int exe_search(const char * commandName, char *** listP, size_t * listSize, bool findAll) {
    if (commandName == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (commandName[0] == '\0') {
        errno = EINVAL;
        return -1;
    }

    if (listP == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (listSize == NULL) {
        errno = EINVAL;
        return -1;
    }

    const char * const PATH = getenv("PATH");

    if (PATH == NULL) {
        return -2;
    }

    if (PATH[0] == '\0') {
        return -3;
    }

    size_t  PATH2Length = strlen(PATH) + 1U;
    char    PATH2[PATH2Length];
    strncpy(PATH2, PATH, PATH2Length);

    struct stat st;

    size_t commandNameLength = strlen(commandName);

    char ** stringArrayList = NULL;
    size_t  stringArrayListSize    = 0U;
    size_t  stringArrayListCapcity = 0U;

    char * PATHItem = strtok(PATH2, ":");

    while (PATHItem != NULL) {
        if ((stat(PATHItem, &st) == 0) && S_ISDIR(st.st_mode)) {
            size_t   fullPathLength = strlen(PATHItem) + commandNameLength + 2U;
            char     fullPath[fullPathLength];
            snprintf(fullPath, fullPathLength, "%s/%s", PATHItem, commandName);

            if (access(fullPath, X_OK) == 0) {
                if (stringArrayListCapcity == stringArrayListSize) {
                    stringArrayListCapcity += 2U;

                    char** paths = (char**)realloc(stringArrayList, stringArrayListCapcity * sizeof(char*));

                    if (paths == NULL) {
                        if (stringArrayList != NULL) {
                            for (size_t i = 0; i < stringArrayListSize; i++) {
                                free(stringArrayList[i]);
                                stringArrayList[i] = NULL;
                            }
                            free(stringArrayList);
                        }
                        errno = ENOMEM;
                        return -1;
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
                    errno = ENOMEM;
                    return -1;
                }

                stringArrayList[stringArrayListSize] = fullPathDup;
                stringArrayListSize += 1U;

                if (!findAll) {
                    break;
                }
            }
        }

        PATHItem = strtok(NULL, ":");
    }

    (*listP)    = stringArrayList;
    (*listSize) = stringArrayListSize;

    return 0;
}

int exe_lookup(const char * commandName, char ** pathP, size_t * pathLength) {
    if (commandName == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (commandName[0] == '\0') {
        errno = EINVAL;
        return -1;
    }

    if (pathP == NULL) {
        errno = EINVAL;
        return -1;
    }

    const char * const PATH = getenv("PATH");

    if (PATH == NULL) {
        return -2;
    }

    if (PATH[0] == '\0') {
        return -3;
    }

    size_t  PATH2Length = strlen(PATH) + 1U;
    char    PATH2[PATH2Length];
    strncpy(PATH2, PATH, PATH2Length);

    struct stat st;

    size_t commandNameLength = strlen(commandName);

    char * PATHItem = strtok(PATH2, ":");

    while (PATHItem != NULL) {
        if ((stat(PATHItem, &st) == 0) && S_ISDIR(st.st_mode)) {
            size_t   fullPathLength = strlen(PATHItem) + commandNameLength + 2U;
            char     fullPath[fullPathLength];
            snprintf(fullPath, fullPathLength, "%s/%s", PATHItem, commandName);

            if (access(fullPath, X_OK) == 0) {
                char * fullPathDup = strdup(fullPath);

                if (fullPathDup == NULL) {
                    errno = ENOMEM;
                    return -1;
                }

                (*pathP) = fullPathDup;

                if (pathLength != NULL) {
                    (*pathLength) = fullPathLength;
                }

                return 0;
            }
        }

        PATHItem = strtok(NULL, ":");
    }

    if (pathLength != NULL) {
        (*pathLength) = 0;
    }

    (*pathP) = NULL;
    return 0;
}

int exe_where(const char * commandName, char buf[], size_t * writtenSize, size_t maxSize) {
    if (commandName == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (commandName[0] == '\0') {
        errno = EINVAL;
        return -1;
    }

    if (buf == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (maxSize == 0U) {
        errno = EINVAL;
        return -1;
    }

    const char * const PATH = getenv("PATH");

    if (PATH == NULL) {
        return -2;
    }

    if (PATH[0] == '\0') {
        return -3;
    }

    size_t  PATH2Length = strlen(PATH) + 1U;
    char    PATH2[PATH2Length];
    strncpy(PATH2, PATH, PATH2Length);

    struct stat st;

    size_t commandNameLength = strlen(commandName);

    char * PATHItem = strtok(PATH2, ":");

    while (PATHItem != NULL) {
        if ((stat(PATHItem, &st) == 0) && S_ISDIR(st.st_mode)) {
            size_t   fullPathLength = strlen(PATHItem) + commandNameLength + 2U;
            char     fullPath[fullPathLength];
            snprintf(fullPath, fullPathLength, "%s/%s", PATHItem, commandName);

            if (access(fullPath, X_OK) == 0) {
                size_t n = (maxSize > fullPathLength) ? fullPathLength : maxSize;

                strncpy(buf, fullPath, n);

                if (writtenSize != NULL) {
                    (*writtenSize) = n;
                }

                return 0;
            }
        }

        PATHItem = strtok(NULL, ":");
    }

    if (writtenSize != NULL) {
        (*writtenSize) = 0;
    }

    return 0;
}
