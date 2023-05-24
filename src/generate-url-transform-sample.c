#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "core/log.h"
#include "ppkg.h"

int ppkg_generate_url_transform_sample() {
    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    ////////////////////////////////////////////////////////////////

    struct stat st;

    size_t ppkgHomeDirLength = userHomeDirLength + 7U;
    char   ppkgHomeDir[ppkgHomeDirLength];
    snprintf(ppkgHomeDir, ppkgHomeDirLength, "%s/.ppkg", userHomeDir);

    if (stat(ppkgHomeDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", ppkgHomeDir);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(ppkgHomeDir, S_IRWXU) != 0) {
            perror(ppkgHomeDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////

    size_t urlTransformSampleFilePathLength = ppkgHomeDirLength + 22U;
    char   urlTransformSampleFilePath[urlTransformSampleFilePathLength];
    snprintf(urlTransformSampleFilePath, urlTransformSampleFilePathLength, "%s/url-transform.sample", ppkgHomeDir);

    FILE * file = fopen(urlTransformSampleFilePath, "w");

    if (file == NULL) {
        perror(urlTransformSampleFilePath);
        return PPKG_ERROR;
    }

    const char * p = ""
        "#!/bin/sh\n"
        "case $1 in\n"
        "    *githubusercontent.com/*)\n"
        "        printf 'https://ghproxy.com/%s\\n' \"$1\"\n"
        "        ;;\n"
        "    https://github.com/*)\n"
        "        printf 'https://ghproxy.com/%s\\n' \"$1\"\n"
        "        ;;\n"
        "    '') printf '%s\\n' \"$0 <URL>, <URL> is unspecified.\" >&2 ;;\n"
        "    *)  printf '%s\\n' \"$1\"\n"
        "esac";

    size_t pSize = strlen(p);

    if (fwrite(p, 1, pSize, file) != pSize || ferror(file)) {
        fclose(file);
        return PPKG_ERROR;
    }

    fclose(file);

    if (chmod(urlTransformSampleFilePath, S_IRWXU) == 0) {
        fprintf(stderr, "%surl-transform sample has been written into %s%s\n\n", COLOR_GREEN, urlTransformSampleFilePath, COLOR_OFF);

        urlTransformSampleFilePath[urlTransformSampleFilePathLength - 9] = '\0';

        fprintf(stderr, "%sYou can rename url-transform.sample to url-transform then edit it to meet your needs.\n\nTo apply this, you should run 'export PPKG_URL_TRANSFORM=%s' in your terminal.\n%s", COLOR_GREEN, urlTransformSampleFilePath, COLOR_OFF);
        return PPKG_OK;
    } else {
        perror(urlTransformSampleFilePath);
        return PPKG_ERROR;
    }
}
