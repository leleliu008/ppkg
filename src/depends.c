#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <curl/curl.h>

#include "ppkg.h"

int ppkg_depends_make_dot_items(const char * packageName, char buff[256]) {
    PPKGFormula * formula = NULL;

    int resultCode = ppkg_formula_parse(packageName, &formula);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    if ((formula->dep_pkg == NULL) || (strcmp(formula->dep_pkg, "") == 0)) {
        ppkg_formula_free(formula);
        return PPKG_OK;
    }

    strcat(buff, "\"");
    strcat(buff, packageName);
    strcat(buff, "\"");
    strcat(buff, " -> { ");

    size_t depPackageNamesLength = strlen(formula->dep_pkg);
    size_t depPackageNamesCopyLength = depPackageNamesLength + 1;
    char   depPackageNamesCopy[depPackageNamesCopyLength];
    memset(depPackageNamesCopy, 0, depPackageNamesCopyLength);
    strcpy(depPackageNamesCopy, formula->dep_pkg);

    size_t index = 0;
    char * depPackageNameList[10];

    char * depPackageName = strtok(depPackageNamesCopy, " ");

    while (depPackageName != NULL) {
        strcat(buff, "\"");
        strcat(buff, depPackageName);
        strcat(buff, "\" ");

        depPackageNameList[index] = depPackageName;
        index++;
        depPackageName = strtok (NULL, " ");
    }

    strcat(buff, "}\n");

    for (size_t i = 0; i < index; i++) {
        resultCode = ppkg_depends_make_dot_items(depPackageNameList[i], buff);

        if (resultCode != PPKG_OK) {
            ppkg_formula_free(formula);
            return resultCode;
        }
    }

    ppkg_formula_free(formula);
    return PPKG_OK;
}

static int ppkg_depends_make_box(const char * dotScriptStr) {
    curl_global_init(CURL_GLOBAL_ALL);

    CURL * curl = curl_easy_init();

    char * dataUrlEncoded = curl_easy_escape(curl, dotScriptStr, strlen(dotScriptStr));

    size_t  urlLength = strlen(dataUrlEncoded) + 66;
    char    url[urlLength];
    memset (url, 0, urlLength);
    sprintf(url, "https://dot-to-ascii.ggerganov.com/dot-to-ascii.php?boxart=1&src=%s", dataUrlEncoded);

    //printf("url=%s\n", url);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    bool verbose = false;
    bool showProgress = false;

    // https://curl.se/libcurl/c/CURLOPT_VERBOSE.html
    if (verbose) {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    } else {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
    }

    // https://curl.se/libcurl/c/CURLOPT_NOPROGRESS.html
    if (showProgress) {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    } else {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    }

    CURLcode curlcode = curl_easy_perform(curl);

    int resultCode = PPKG_OK;

    if (curlcode != CURLE_OK) {
        fprintf(stderr, "%s\n", curl_easy_strerror(curlcode));
        resultCode = PPKG_NETWORK_ERROR;
    }

    curl_free(dataUrlEncoded);
    curl_easy_cleanup(curl);

    curl_global_cleanup();

    return resultCode;
}

static int ppkg_depends_make_png(const char * dotScriptStr) {
    size_t  cmdLength = strlen(dotScriptStr) + 22;
    char    cmd[cmdLength];
    memset( cmd, 0, cmdLength);
    sprintf(cmd, "dot -Tpng <<EOF\n%s\nEOF", dotScriptStr);

    if (system(cmd) == 0) {
        return PPKG_OK;
    } else {
        fprintf(stderr, "command not found: dot\n");
        return PPKG_ERROR;
    }
}

static int ppkg_depends_make_svg(const char * dotScriptStr) {
    size_t  cmdLength = strlen(dotScriptStr) + 22;
    char    cmd[cmdLength];
    memset( cmd, 0, cmdLength);
    sprintf(cmd, "dot -Tsvg <<EOF\n%s\nEOF", dotScriptStr);

    if (system(cmd) == 0) {
        return PPKG_OK;
    } else {
        fprintf(stderr, "command not found: dot\n");
        return PPKG_ERROR;
    }
}

int ppkg_depends(const char * packageName, PPKGDependsOutputFormat outputFormat) {
    int resultCode = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    char buff[256] = {0};

    resultCode = ppkg_depends_make_dot_items(packageName, buff);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    size_t n = strlen(buff);

    if (n == 0) {
        return PPKG_OK;
    }

           if (outputFormat == PPKGDependsOutputFormat_DOT) {
        printf("digraph G {\n%s}\n", buff);
        return PPKG_OK;
    } else if (outputFormat == PPKGDependsOutputFormat_BOX) {
        size_t  dotScriptStrLength = n + 15;
        char    dotScriptStr[dotScriptStrLength];
        memset( dotScriptStr, 0, dotScriptStrLength);
        sprintf(dotScriptStr, "digraph G {\n%s}", buff);

        return ppkg_depends_make_box(dotScriptStr);
    } else if (outputFormat == PPKGDependsOutputFormat_PNG) {
        size_t  dotScriptStrLength = n + 15;
        char    dotScriptStr[dotScriptStrLength];
        memset( dotScriptStr, 0, dotScriptStrLength);
        sprintf(dotScriptStr, "digraph G {\n%s}", buff);

        return ppkg_depends_make_png(dotScriptStr);
    } else if (outputFormat == PPKGDependsOutputFormat_SVG) {
        size_t  dotScriptStrLength = n + 15;
        char    dotScriptStr[dotScriptStrLength];
        memset( dotScriptStr, 0, dotScriptStrLength);
        sprintf(dotScriptStr, "digraph G {\n%s}", buff);

        return ppkg_depends_make_svg(dotScriptStr);
    } else {
        return PPKG_ERROR;
    }
}
