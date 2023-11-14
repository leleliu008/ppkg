#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
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
    size_t  stringArrayListCapacity = 0U;

    char * PATHItem = strtok(PATH2, ":");

    while (PATHItem != NULL) {
        if ((stat(PATHItem, &st) == 0) && S_ISDIR(st.st_mode)) {
            size_t fullPathCapacity = strlen(PATHItem) + commandNameLength + 2U;
            char   fullPath[fullPathCapacity];

            int ret = snprintf(fullPath, fullPathCapacity, "%s/%s", PATHItem, commandName);

            if (ret < 0) {
                return -1;
            }

            if (access(fullPath, X_OK) == 0) {
                if (stringArrayListCapacity == stringArrayListSize) {
                    stringArrayListCapacity += 2U;

                    char** paths = (char**)realloc(stringArrayList, stringArrayListCapacity * sizeof(char*));

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
            size_t fullPathCapacity = strlen(PATHItem) + commandNameLength + 2U;
            char   fullPath[fullPathCapacity];

            int ret = snprintf(fullPath, fullPathCapacity, "%s/%s", PATHItem, commandName);

            if (ret < 0) {
                return -1;
            }

            if (access(fullPath, X_OK) == 0) {
                char * fullPathDup = strdup(fullPath);

                if (fullPathDup == NULL) {
                    errno = ENOMEM;
                    return -1;
                }

                (*pathP) = fullPathDup;

                if (pathLength != NULL) {
                    (*pathLength) = ret;
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

int exe_where(const char * commandName, char buf[], size_t bufSize, size_t * writtenSize) {
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

    if (bufSize == 0U) {
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

    char   pathBuf[PATH_MAX];

    while (PATHItem != NULL) {
        if ((stat(PATHItem, &st) == 0) && S_ISDIR(st.st_mode)) {
            size_t max = strlen(PATHItem) + commandNameLength + 2U;

            int ret = snprintf(pathBuf, max, "%s/%s", PATHItem, commandName);

            if (ret < 0) {
                return -1;
            }

            size_t pathLength = ret;

            if (access(pathBuf, X_OK) == 0) {
                size_t m = bufSize - 1U;
                size_t n = (m > pathLength) ? pathLength : m;

                strncpy(buf, pathBuf, n);

                buf[n] = '\0';

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
