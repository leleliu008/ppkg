#include <stdio.h>
#include <string.h>

#include "core/url-transform.h"
#include "core/http.h"

#include "ppkg.h"

int ppkg_http_fetch_to_file(const char * url, const char * outputFilePath, const bool verbose, const bool showProgress) {
    const char * urlTransform = getenv("PPKG_URL_TRANSFORM");

    if (urlTransform == NULL || urlTransform[0] == '\0') {
        int ret = http_fetch_to_file(url, outputFilePath, verbose, showProgress);

        if (ret == -1) {
            perror(outputFilePath);
            return PPKG_ERROR;
        }

        if (ret > 0) {
            return PPKG_ERROR_NETWORK_BASE + ret;
        }

        return PPKG_OK;
    } else {
        if (verbose) {
            fprintf(stderr, "\nyou have set PPKG_URL_TRANSFORM=%s\n", urlTransform);
            fprintf(stderr, "transform from: %s\n", url);
        }

        char   transformedUrl[1025] = {0};
        size_t transformedUrlLength = 0U;

        if (url_transform(urlTransform, url, transformedUrl, 1024, &transformedUrlLength, true) != 0) {
            perror(urlTransform);
            return PPKG_ERROR;
        }

        if (transformedUrlLength == 0U) {
            if (verbose) {
                fprintf(stderr, "a new url was expected to be output, but it was not.\n");
            }
            return PPKG_ERROR;
        }

        if (verbose) {
            fprintf(stderr, "transform   to: %s\n", transformedUrl);
        }

        int ret = http_fetch_to_file(transformedUrl, outputFilePath, verbose, showProgress);

        if (ret == -1) {
            perror(outputFilePath);
            return PPKG_ERROR;
        }

        if (ret > 0) {
            return PPKG_ERROR_NETWORK_BASE + ret;
        }

        return PPKG_OK;
    }
}

int ppkg_http_fetch_to_stream(const char * url, FILE * stream, const bool verbose, const bool showProgress) {
    const char * urlTransform = getenv("PPKG_URL_TRANSFORM");

    if (urlTransform == NULL || urlTransform[0] == '\0') {
        int ret = http_fetch_to_stream(url, stream, verbose, showProgress);

        if (ret == -1) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (ret > 0) {
            return PPKG_ERROR_NETWORK_BASE + ret;
        }

        return PPKG_OK;
    } else {
        if (verbose) {
            fprintf(stderr, "\nyou have set PPKG_URL_TRANSFORM=%s\n", urlTransform);
            fprintf(stderr, "transform from: %s\n", url);
        }

        char   transformedUrl[1025] = {0};
        size_t transformedUrlLength = 0U;

        if (url_transform(urlTransform, url, transformedUrl, 1024, &transformedUrlLength, true) != 0) {
            perror(urlTransform);
            return PPKG_ERROR;
        }

        if (transformedUrlLength == 0U) {
            if (verbose) {
                fprintf(stderr, "a new url was expected to be output, but it was not.\n");
            }
            return PPKG_ERROR;
        }

        if (verbose) {
            fprintf(stderr, "transform   to: %s\n", transformedUrl);
        }

        int ret = http_fetch_to_stream(transformedUrl, stream, verbose, showProgress);

        if (ret == -1) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (ret > 0) {
            return PPKG_ERROR_NETWORK_BASE + ret;
        }

        return PPKG_OK;
    }
}
