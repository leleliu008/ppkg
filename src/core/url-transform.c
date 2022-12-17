#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define URL_TRANSFORM_OK                0
#define URL_TRANSFORM_ERROR             101
#define URL_TRANSFORM_RUN_EMPTY_RESULT  102
#define URL_TRANSFORM_ENV_IS_NOT_SET    103
#define URL_TRANSFORM_ENV_VALUE_IS_EMPTY      104
#define URL_TRANSFORM_ENV_VALUE_PATH_NOT_EXIST 105

int url_transform(const char * url, char ** out) {
    char * urlTransformCommandPath = getenv("URL_TRANSFORM");

    if (urlTransformCommandPath == NULL) {
        return URL_TRANSFORM_ENV_IS_NOT_SET;
    }

    if (strcmp(urlTransformCommandPath, "") == 0) {
        return URL_TRANSFORM_ENV_VALUE_IS_EMPTY;
    }

    struct stat sb;

    if ((stat(urlTransformCommandPath, &sb) == 0) && (S_ISREG(sb.st_mode) || S_ISLNK(sb.st_mode))) {
        ;
    } else {
        return URL_TRANSFORM_ENV_VALUE_PATH_NOT_EXIST;
    }

    size_t urlLength = strlen(url);

    size_t  cmdLength = urlLength + strlen(urlTransformCommandPath) + 2;
    char    cmd[cmdLength];
    memset( cmd, 0, cmdLength);
    sprintf(cmd, "%s %s", urlTransformCommandPath, url);

    FILE * file = popen(cmd, "r");

    if (file == NULL) {
        perror(cmd);
        return URL_TRANSFORM_ERROR;
    }

    size_t capcity = 256;
    size_t size    = 0;
    char * result = NULL;
    char   c;

    for (;;) {
        c = fgetc(file);

        if (c == EOF) {
            break;
        }

        if (c == '\n') {
            break;
        }

        if (size == 0) {
            result = (char*)calloc(capcity, sizeof(char));
        } else if (capcity == size) {
            capcity += capcity;
            result = (char*)realloc(result, capcity);
        }

        result[size] = c;
        size++;
    }

    pclose(file);

    if (result == NULL) {
        return URL_TRANSFORM_RUN_EMPTY_RESULT;
    } else {
        (*out) = result;
        return URL_TRANSFORM_OK;
    }
}
