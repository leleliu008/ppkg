#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "core/http.h"
#include "core/url.h"

#include "ppkg.h"

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
            memset(&p[*outSize], 0, 256U + bufLength + 1U);
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
} DIRectedPath;

static int ppkg_depends2(const char * packageName, PPKGDependsOutputType outputType, const char * outputFilePath) {
    int ret = PPKG_OK;

    ////////////////////////////////////////////////////////////////

    char * p = NULL;
    size_t pSize = 0;
    size_t pCapcity = 0;

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
                PPKGPackage ** p = (PPKGPackage**)realloc(packageSet, (packageSetCapcity + 10U) * sizeof(PPKGPackage*));

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

        size_t   bufLength = strlen(packageName) + 12U;
        char     buf[bufLength];
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

            ret = string_append(&p, &pSize, &pCapcity, buf, bufLength - 1U);

            if (ret != PPKG_OK) {
                goto finalize;
            }

            ////////////////////////////////////////////////////////////////

            ////////////////////////////////////////////////////////////////

            if (packageNameStackSize == packageNameStackCapcity) {
                char ** p = (char**)realloc(packageNameStack, (packageNameStackCapcity + 10U) * sizeof(char*));

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

    size_t   dotScriptStrLength = pSize + 14U;
    char     dotScriptStr[dotScriptStrLength];
    snprintf(dotScriptStr, dotScriptStrLength, "digraph G {\n%s}", p);

    free(p);

    dotScriptStrLength--;

    ////////////////////////////////////////////////////////////////

    if (outputType == PPKGDependsOutputType_DOT) {
        if (outputFilePath == NULL) {
            printf("%s\n", dotScriptStr);
            return PPKG_OK;
        }
    }

    ////////////////////////////////////////////////////////////////

    char   ppkgHomeDIR[256] = {0};
    size_t ppkgHomeDIRLength;

    ret = ppkg_home_dir(ppkgHomeDIR, 255, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    struct stat st;

    size_t   ppkgRunDIRLength = ppkgHomeDIRLength + 5U;
    char     ppkgRunDIR[ppkgRunDIRLength];
    snprintf(ppkgRunDIR, ppkgRunDIRLength, "%s/run", ppkgHomeDIR);

    if (lstat(ppkgRunDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(ppkgRunDIR) != 0) {
                perror(ppkgRunDIR);
                return PPKG_ERROR;
            }

            if (mkdir(ppkgRunDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(ppkgRunDIR);
                    return PPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(ppkgRunDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(ppkgRunDIR);
                return PPKG_ERROR;
            }
        }
    }

    ////////////////////////////////////////////////////////////////

    size_t   sessionDIRLength = ppkgRunDIRLength + 20U;
    char     sessionDIR[sessionDIRLength];
    snprintf(sessionDIR, sessionDIRLength, "%s/%d", ppkgRunDIR, getpid());

    if (lstat(sessionDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            ret = ppkg_rm_r(sessionDIR, false);

            if (ret != PPKG_OK) {
                return ret;
            }

            if (mkdir(sessionDIR, S_IRWXU) != 0) {
                perror(sessionDIR);
                return PPKG_ERROR;
            }
        } else {
            if (unlink(sessionDIR) != 0) {
                perror(sessionDIR);
                return PPKG_ERROR;
            }

            if (mkdir(sessionDIR, S_IRWXU) != 0) {
                perror(sessionDIR);
                return PPKG_ERROR;
            }
        }
    } else {
        if (mkdir(sessionDIR, S_IRWXU) != 0) {
            perror(sessionDIR);
            return PPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////

    if (outputType == PPKGDependsOutputType_BOX) {
        size_t urlEncodedBufLength = 3 * dotScriptStrLength + 1U;
        char   urlEncodedBuf[urlEncodedBufLength];
        memset(urlEncodedBuf, 0, urlEncodedBufLength);

        size_t urlEncodedRealLength = 0;

        if (url_encode(urlEncodedBuf, &urlEncodedRealLength, (unsigned char *)dotScriptStr, dotScriptStrLength, true) != 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        size_t   urlLength = urlEncodedRealLength + 66U;
        char     url[urlLength];
        snprintf(url, urlLength, "https://dot-to-ascii.ggerganov.com/dot-to-ascii.php?boxart=1&src=%s", urlEncodedBuf);

        //printf("url=%s\n", url);

        if (outputFilePath == NULL) {
            int ret = http_fetch_to_stream(url, stdout, false, false);

            if (ret == -1) {
                return PPKG_ERROR;
            }

            if (ret > 0) {
                return PPKG_ERROR_NETWORK_BASE + ret;
            }

            return PPKG_OK;
        } else {
            size_t   boxFilePathLength = sessionDIRLength + 18U;
            char     boxFilePath[boxFilePathLength];
            snprintf(boxFilePath, boxFilePathLength, "%s/dependencies.box", sessionDIR);

            int ret = http_fetch_to_file(url, boxFilePath, false, false);

            if (ret == -1) {
                perror(outputFilePath);
                return PPKG_ERROR;
            }

            if (ret > 0) {
                return PPKG_ERROR_NETWORK_BASE + ret;
            } else {
                return ppkg_rename_or_copy_file(boxFilePath, outputFilePath);
            }
        }
    } 

    ////////////////////////////////////////////////////////////////

    size_t   dotFilePathLength = sessionDIRLength + 18U;
    char     dotFilePath[dotFilePathLength];
    snprintf(dotFilePath, dotFilePathLength, "%s/dependencies.dot", sessionDIR);

    int dotFD = open(dotFilePath, O_CREAT | O_WRONLY | O_TRUNC, 0666);

    if (dotFD == -1) {
        perror(dotFilePath);
        return PPKG_ERROR;
    }

    ret = write(dotFD, dotScriptStr, dotScriptStrLength);

    if (ret == -1) {
        perror(dotFilePath);
        close(dotFD);
        return PPKG_ERROR;
    }

    close(dotFD);

    if ((size_t)ret != dotScriptStrLength) {
        fprintf(stderr, "not fully written: %s\n", dotFilePath);
        return PPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////

    if (outputType == PPKGDependsOutputType_DOT) {
        return ppkg_rename_or_copy_file(dotFilePath, outputFilePath);
    }

    ////////////////////////////////////////////////////////////////

    size_t   tmpFilePathLength = sessionDIRLength + 18U;
    char     tmpFilePath[tmpFilePathLength];
    snprintf(tmpFilePath, tmpFilePathLength, "%s/dependencies.tmp", sessionDIR);

    ////////////////////////////////////////////////////////////////

    int pid = fork();

    if (pid == -1) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (pid == 0) {
        if (outputType == PPKGDependsOutputType_PNG) {
            execlp("dot", "dot", "-Tpng", "-o", tmpFilePath, dotFilePath, NULL);
        } else if (outputType == PPKGDependsOutputType_SVG) {
            execlp("dot", "dot", "-Tsvg", "-o", tmpFilePath, dotFilePath, NULL);
        }

        perror("dot");
        exit(255);
    } else {
        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (childProcessExitStatusCode == 0) {
            return ppkg_rename_or_copy_file(tmpFilePath, outputFilePath);
        }

        const char * type;

        if (outputType == PPKGDependsOutputType_PNG) {
            type = "-Tpng";
        } else if (outputType == PPKGDependsOutputType_SVG) {
            type = "-Tsvg";
        }

        if (WIFEXITED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command 'dot %s -o %s %s' exit with status code: %d\n", type, tmpFilePath, dotFilePath, WEXITSTATUS(childProcessExitStatusCode));
        } else if (WIFSIGNALED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command 'dot %s -o %s %s' killed by signal: %d\n", type, tmpFilePath, dotFilePath, WTERMSIG(childProcessExitStatusCode));
        } else if (WIFSTOPPED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command 'dot %s -o %s %s' stopped by signal: %d\n", type, tmpFilePath, dotFilePath, WSTOPSIG(childProcessExitStatusCode));
        }

        return PPKG_ERROR;
    }
}

int ppkg_depends(const char * packageName, PPKGDependsOutputType outputType, const char * outputPath) {
    char * outputFilePath = NULL;

    if (outputPath != NULL) {
        struct stat st;

        if (stat(outputPath, &st) == 0 && S_ISDIR(st.st_mode)) {
            size_t outputFilePathLength = strlen(outputPath) + strlen(packageName) + 20U;

            outputFilePath = (char*) malloc(outputFilePathLength);

            if (outputFilePath == NULL) {
                return PPKG_ERROR_MEMORY_ALLOCATE;
            }

            const char * outputFileNameSuffix;

            switch (outputType) {
                case PPKGDependsOutputType_DOT: outputFileNameSuffix = "dot"; break;
                case PPKGDependsOutputType_BOX: outputFileNameSuffix = "box"; break;
                case PPKGDependsOutputType_SVG: outputFileNameSuffix = "svg"; break;
                case PPKGDependsOutputType_PNG: outputFileNameSuffix = "png"; break;
            }

            snprintf(outputFilePath, outputFilePathLength, "%s/%s-dependencies.%s", outputPath, packageName, outputFileNameSuffix);
        } else {
            size_t outputPathLength = strlen(outputPath);

            if (outputPath[outputPathLength - 1] == '/') {
                size_t outputFilePathLength = strlen(outputPath) + strlen(packageName) + 20U;

                outputFilePath = (char*) malloc(outputFilePathLength);

                if (outputFilePath == NULL) {
                    return PPKG_ERROR_MEMORY_ALLOCATE;
                }

                const char * outputFileNameSuffix;

                switch (outputType) {
                    case PPKGDependsOutputType_DOT: outputFileNameSuffix = "dot"; break;
                    case PPKGDependsOutputType_BOX: outputFileNameSuffix = "box"; break;
                    case PPKGDependsOutputType_SVG: outputFileNameSuffix = "svg"; break;
                    case PPKGDependsOutputType_PNG: outputFileNameSuffix = "png"; break;
                }

                snprintf(outputFilePath, outputFilePathLength, "%s%s-dependencies.%s", outputPath, packageName, outputFileNameSuffix);
            } else {
                outputFilePath = strdup(outputPath);

                if (outputFilePath == NULL) {
                    return PPKG_ERROR_MEMORY_ALLOCATE;
                }
            }
        }
    }

    int ret = ppkg_depends2(packageName, outputType, outputFilePath);

    free(outputFilePath);

    return ret;
}
