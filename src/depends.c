#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <curl/curl.h>

#include "ppkg.h"

static size_t write_callback(void * ptr, size_t size, size_t nmemb, void * stream) {
    return fwrite(ptr, size, nmemb, (FILE *)stream);
}

static int ppkg_depends_make_box(const char * dotScriptStr, const char * outputFilePath) {
    curl_global_init(CURL_GLOBAL_ALL);

    CURL * curl = curl_easy_init();

    char * dataUrlEncoded = curl_easy_escape(curl, dotScriptStr, strlen(dotScriptStr));

    size_t urlLength = strlen(dataUrlEncoded) + 66U;
    char   url[urlLength];
    snprintf(url, urlLength, "https://dot-to-ascii.ggerganov.com/dot-to-ascii.php?boxart=1&src=%s", dataUrlEncoded);

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

    CURLcode curlcode = CURLE_OK;

    int ret = PPKG_OK;

    if (outputFilePath != NULL) {
        FILE * outputFile = fopen(outputFilePath, "wb");

        if (outputFile == NULL) {
            perror(outputFilePath);
            ret = PPKG_ERROR;
            goto finalize;
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, outputFile);
    }

    curlcode = curl_easy_perform(curl);

finalize:

    if (curlcode != CURLE_OK) {
        fprintf(stderr, "%s\n", curl_easy_strerror(curlcode));
        ret = PPKG_ERROR_NETWORK_BASE + curlcode;
    }

    curl_free(dataUrlEncoded);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return ret;
}

static int ppkg_depends_make_xxx(const char * dotScriptStr, size_t len, const char * tOption, const char * oOption) {
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

    size_t ppkgTmpDirLength = ppkgHomeDirLength + 5U;
    char   ppkgTmpDir[ppkgTmpDirLength];
    snprintf(ppkgTmpDir, ppkgTmpDirLength, "%s/tmp", ppkgHomeDir);

    if (stat(ppkgTmpDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", ppkgTmpDir);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(ppkgTmpDir, S_IRWXU) != 0) {
            perror(ppkgTmpDir);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////

    char ts[11] = {0};
    snprintf(ts, 11, "%ld", time(NULL));

    size_t filepathLength = ppkgTmpDirLength + strlen(ts) + 6U;
    char   filepath[filepathLength];
    snprintf(filepath, filepathLength, "%s/%s.dot", ppkgTmpDir, ts);

    FILE * file = fopen(filepath, "w");

    if (file == NULL) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (fwrite(dotScriptStr, 1, len, file) != len || ferror(file)) {
        perror(filepath);
        fclose(file);
        return PPKG_ERROR;
    }

    fclose(file);

    if (oOption == NULL) {
        execlp("dot", "dot", tOption,          filepath, NULL);
    } else {
        execlp("dot", "dot", tOption, oOption, filepath, NULL);
    }

    perror("dot");

    return PPKG_ERROR;
}

static int string_append(char ** outP, size_t * outSize, size_t * outCapcity, const char * buf, size_t bufLength) {
    size_t  oldCapcity = (*outCapcity);
    size_t needCapcity = oldCapcity + bufLength + 1U;

    if (needCapcity > oldCapcity) {
        size_t newCapcity = needCapcity + 256U;

        char * p = (char*)realloc(*outP, newCapcity * sizeof(char));

        if (p == NULL) {
            free(*outP);
            return PPKG_ERROR_MEMORY_ALLOCATE;
        } else {
            (*outP) = p;
            (*outCapcity) = newCapcity;
            memset(&p[*outSize], 0, 256 + bufLength + 1);
        }
    }

    strncat(*outP, buf, bufLength);
        
    (*outSize) += bufLength;

    return PPKG_OK;
}

typedef struct {
    char * packageName;
    PPKGFormula * formula;
} PPKGPackage;

typedef struct {
    char * nodeList;
    size_t nodeListSize;
    size_t nodeListCapcity;
} DirectedPath;

int ppkg_depends(const char * packageName, PPKGDependsOutputType outputType, const char * outputFilePath) {
    int ret = PPKG_OK;

    ////////////////////////////////////////////////////////////////

    char * p = NULL;
    size_t pSize = 0;
    size_t pCapcity = 0;

    ////////////////////////////////////////////////////////////////

    size_t          directedPathArrayListCapcity = 0;
    size_t          directedPathArrayListSize    = 0;
    DirectedPath ** directedPathArrayList        = NULL;

    ////////////////////////////////////////////////////////////////

    size_t         packageSetCapcity = 0;
    size_t         packageSetSize    = 0;
    PPKGPackage ** packageSet        = NULL;

    ////////////////////////////////////////////////////////////////

    size_t   packageNameStackCapcity = 1;
    size_t   packageNameStackSize    = 0;
    char * * packageNameStack = (char**)calloc(1, sizeof(char*));

    if (packageNameStack == NULL) {
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    packageNameStack[0] = strdup(packageName);

    if (packageNameStack[0] == NULL) {
        free(packageNameStack);
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    packageNameStackSize = 1;

    ////////////////////////////////////////////////////////////////

    while (packageNameStackSize > 0) {
        char * packageName = packageNameStack[packageNameStackSize - 1];
        packageNameStack[packageNameStackSize - 1] = NULL;
        packageNameStackSize--;

        PPKGFormula * formula = NULL;

        for (size_t i = 0; i < packageSetSize; i++) {
            if (strcmp(packageSet[i]->packageName, packageName) == 0) {
                formula = packageSet[i]->formula;
                free(packageName);
                packageName = packageSet[i]->packageName;
                break;
            }
        }

        if (formula == NULL) {
            ret = ppkg_formula_lookup(packageName, &formula);

            if (ret != PPKG_OK) {
                free(packageName);
                goto finalize;
            }

            if (packageSetSize == packageSetCapcity) {
                PPKGPackage ** p = (PPKGPackage**)realloc(packageSet, (packageSetCapcity + 10) * sizeof(PPKGPackage*));

                if (p == NULL) {
                    free(packageName);
                    ppkg_formula_free(formula);
                    ret = PPKG_ERROR_MEMORY_ALLOCATE;
                    goto finalize;
                }

                memset(p + packageSetCapcity, 0, 10);

                packageSet = p;
                packageSetCapcity += 10;
            }

            PPKGPackage * package = (PPKGPackage*)malloc(sizeof(PPKGPackage));

            if (package == NULL) {
                free(packageName);
                ppkg_formula_free(formula);
                ret = PPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            package->formula = formula;
            package->packageName = strdup(packageName);

            packageSet[packageSetSize] = package;
            packageSetSize++;

            if (package->packageName == NULL) {
                free(packageName);
                ret = PPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            free(packageName);
            packageName = package->packageName;
        }

        if (formula->dep_pkg == NULL) {
            continue;
        }

        ////////////////////////////////////////////////////////////////

        size_t bufLength = strlen(packageName) + 12U;
        char   buf[bufLength];
        snprintf(buf, bufLength, "    \"%s\" -> {", packageName);

        ret = string_append(&p, &pSize, &pCapcity, buf, bufLength - 1);

        if (ret != PPKG_OK) {
            goto finalize;
        }

        ////////////////////////////////////////////////////////////////

        size_t depPackageNamesLength = strlen(formula->dep_pkg);

        size_t depPackageNamesCopyLength = depPackageNamesLength + 1U;
        char   depPackageNamesCopy[depPackageNamesCopyLength];
        strncpy(depPackageNamesCopy, formula->dep_pkg, depPackageNamesCopyLength);

        char * depPackageName = strtok(depPackageNamesCopy, " ");

        while (depPackageName != NULL) {
            if (strcmp(depPackageName, packageName) == 0) {
                fprintf(stderr, "package '%s' depends itself.\n", packageName);
                ret = PPKG_ERROR;
                goto finalize;
            }

            ////////////////////////////////////////////////////////////////

            size_t  bufLength = strlen(depPackageName) + 4U;
            char    buf[bufLength];
            snprintf(buf, bufLength, " \"%s\"", depPackageName);

            ret = string_append(&p, &pSize, &pCapcity, buf, bufLength - 1);

            if (ret != PPKG_OK) {
                goto finalize;
            }

            ////////////////////////////////////////////////////////////////

            ////////////////////////////////////////////////////////////////

            if (packageNameStackSize == packageNameStackCapcity) {
                char ** p = (char**)realloc(packageNameStack, (packageNameStackCapcity + 10) * sizeof(char*));

                if (p == NULL) {
                    ret = PPKG_ERROR_MEMORY_ALLOCATE;
                    goto finalize;
                }

                memset(p + packageNameStackCapcity, 0, 10);

                packageNameStack = p;
                packageNameStackCapcity += 10;
            }

            char * p = strdup(depPackageName);

            if (p == NULL) {
                ret = PPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            packageNameStack[packageNameStackSize] = p;
            packageNameStackSize++;

            depPackageName = strtok (NULL, " ");
        }

        ret = string_append(&p, &pSize, &pCapcity, " }\n", 3);
    }

finalize:
    for (size_t i = 0; i < packageNameStackSize; i++) {
        free(packageNameStack[i]);
        packageNameStack[i] = NULL;
    }

    free(packageNameStack);
    packageNameStack = NULL;

    //////////////////////////////////////////////////

    for (size_t i = 0; i < packageSetSize; i++) {
        free(packageSet[i]->packageName);
        ppkg_formula_free(packageSet[i]->formula);

        packageSet[i]->formula = NULL;
        packageSet[i]->packageName = NULL;

        free(packageSet[i]);
        packageSet[i] = NULL;
    }

    free(packageSet);
    packageSet = NULL;

    //////////////////////////////////////////////////

    if (ret != PPKG_OK) {
        free(p);
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    if (pSize == 0) {
        return PPKG_OK;
    }

    ////////////////////////////////////////////////////////////////

    if (outputFilePath != NULL && outputFilePath[0] == '\0') {
        outputFilePath = NULL;
    }

    ////////////////////////////////////////////////////////////////

    if (outputType == PPKGDependsOutputType_DOT) {
        if (outputFilePath == NULL) {
            printf("digraph G {\n%s}\n", p);
            free(p);
            return PPKG_OK;
        } else {
            FILE * outputFile = fopen(outputFilePath, "wb");

            if (outputFile == NULL) {
                perror(outputFilePath);
                free(p);
                return PPKG_ERROR;
            }

            fprintf(outputFile, "digraph G {\n%s}\n", p);

            free(p);

            if (ferror(outputFile)) {
                perror(outputFilePath);
                fclose(outputFile);
                return PPKG_ERROR;
            } else {
                fclose(outputFile);
                return PPKG_OK;
            }
        }
    } else if (outputType == PPKGDependsOutputType_BOX) {
        size_t dotScriptStrLength = pSize + 14U;
        char   dotScriptStr[dotScriptStrLength];
        snprintf(dotScriptStr, dotScriptStrLength, "digraph G {\n%s}", p);

        free(p);

        return ppkg_depends_make_box(dotScriptStr, outputFilePath);
    } else if (outputType == PPKGDependsOutputType_PNG) {
        size_t dotScriptStrLength = pSize + 14U;
        char   dotScriptStr[dotScriptStrLength];
        snprintf(dotScriptStr, dotScriptStrLength, "digraph G {\n%s}", p);

        free(p);

        if (outputFilePath == NULL) {
            return ppkg_depends_make_xxx(dotScriptStr, dotScriptStrLength - 1, "-Tpng", NULL);
        } else {
            size_t oOptionLength=strlen(outputFilePath) + 3U;
            char   oOption[oOptionLength];
            snprintf(oOption, oOptionLength, "-o%s", outputFilePath);

            return ppkg_depends_make_xxx(dotScriptStr, dotScriptStrLength - 1, "-Tpng", oOption);
        }
    } else if (outputType == PPKGDependsOutputType_SVG) {
        size_t dotScriptStrLength = pSize + 14U;
        char   dotScriptStr[dotScriptStrLength];
        snprintf(dotScriptStr, dotScriptStrLength, "digraph G {\n%s}", p);

        free(p);

        if (outputFilePath == NULL) {
            return ppkg_depends_make_xxx(dotScriptStr, dotScriptStrLength - 1, "-Tsvg", NULL);
        } else {
            size_t oOptionLength=strlen(outputFilePath) + 3U;
            char   oOption[oOptionLength];
            snprintf(oOption, oOptionLength, "-o%s", outputFilePath);

            return ppkg_depends_make_xxx(dotScriptStr, dotScriptStrLength - 1, "-Tsvg", oOption);
        }
    } else {
        free(p);

        return PPKG_ERROR;
    }
}
