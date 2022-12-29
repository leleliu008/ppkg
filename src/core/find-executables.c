#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "find-executables.h"

int find_executables(ExecuablePathList * pathList, const char * commandName, bool findAll) {
    if (pathList == NULL) {
        return FIND_EXECUTABLES_ARG_INVALID;
    }

    if ((commandName == NULL) || (strcmp(commandName, "") == 0)) {
        return FIND_EXECUTABLES_ARG_INVALID;
    }

    char * PATH = getenv("PATH");

    if ((PATH == NULL) || (strcmp(PATH, "") == 0)) {
        return FIND_EXECUTABLES_ENV_PATH_NO_VALUE;
    }

    size_t PATH2Length = strlen(PATH) + 1;
    char   PATH2[PATH2Length];
    memset(PATH2, 0, PATH2Length);
    strcpy(PATH2, PATH);

    size_t commandNameLength = strlen(commandName);

    size_t capcity = 0;

    char * PATHItem = strtok(PATH2, ":");

    while (PATHItem != NULL) {
        struct stat st;

        if ((stat(PATHItem, &st) == 0) && S_ISDIR(st.st_mode)) {
            size_t  fullPathLength = strlen(PATHItem) + commandNameLength + 2;
            char    fullPath[fullPathLength];
            memset( fullPath, 0, fullPathLength);
            sprintf(fullPath, "%s/%s", PATHItem, commandName);

            if (access(fullPath, X_OK) == 0) {
                if (pathList->size == capcity) {
                    capcity += 2;
                    pathList->paths = (char**)realloc(pathList->paths, capcity * sizeof(char*));

                    if (pathList->paths == NULL) {
                        return FIND_EXECUTABLES_ALLOCATE_MEMORY_FAILED;
                    }
                }

                pathList->paths[pathList->size] = strdup(fullPath);

                if (pathList->paths[pathList->size] == NULL) {
                    return FIND_EXECUTABLES_ALLOCATE_MEMORY_FAILED;
                }

                pathList->size += 1;

                if (!findAll) {
                    break;
                }
            }
        }

        PATHItem = strtok(NULL, ":");
    }

    if (pathList->size == 0) {
        return FIND_EXECUTABLES_NOT_FOUND;
    } else {
        return FIND_EXECUTABLES_FOUND;
    }
}
