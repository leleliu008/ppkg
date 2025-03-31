#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "core/http.h"
#include "core/url.h"

#include "ppkg.h"

static int string_append(char ** outP, size_t * outSize, size_t * outCapacity, const char * buf, size_t bufLength) {
    size_t  oldCapacity = (*outCapacity);
    size_t needCapacity = oldCapacity + bufLength + 1U;

    if (needCapacity > oldCapacity) {
        size_t newCapacity = needCapacity + 256U;

        char * p = (char*)realloc(*outP, newCapacity * sizeof(char));

        if (p == NULL) {
            free(*outP);
            return PPKG_ERROR_MEMORY_ALLOCATE;
        } else {
            (*outP) = p;
            (*outCapacity) = newCapacity;
            memset(&p[*outSize], 0, 256U + bufLength + 1U);
        }
    }

    strncat(*outP, buf, bufLength);
        
    (*outSize) += bufLength;

    return PPKG_OK;
}

static int get_output_file_path(char outputFilePath[PATH_MAX], const char * packageName, PPKGDependsOutputType outputType, const char * outputPath) {
    const char * outputFileNameSuffix;

    switch (outputType) {
        case PPKGDependsOutputType_D2 : outputFileNameSuffix = "d2" ; break;
        case PPKGDependsOutputType_DOT: outputFileNameSuffix = "dot"; break;
        case PPKGDependsOutputType_BOX: outputFileNameSuffix = "box"; break;
        case PPKGDependsOutputType_SVG: outputFileNameSuffix = "svg"; break;
        case PPKGDependsOutputType_PNG: outputFileNameSuffix = "png"; break;
    }

    int ret;

    if (outputPath == NULL || outputPath[0] == '\0') {
        ret = snprintf(outputFilePath, strlen(packageName) + 20U, "%s-dependencies.%s", packageName, outputFileNameSuffix);
    } else {
        size_t outputPathLength = strlen(outputPath);

        if (strcmp(outputPath, ".") == 0) {
           ret = snprintf(outputFilePath, strlen(packageName) + 20U, "%s-dependencies.%s", packageName, outputFileNameSuffix);
        } else if (strcmp(outputPath, "..") == 0) {
           ret = snprintf(outputFilePath, strlen(packageName) + 23U, "../%s-dependencies.%s", packageName, outputFileNameSuffix);
        } else if (outputPath[outputPathLength - 1] == '/') {
           ret = snprintf(outputFilePath, strlen(outputPath) + strlen(packageName) + 20U, "%s%s-dependencies.%s", outputPath, packageName, outputFileNameSuffix);
        } else {
            ret = 0;
            size_t n = strlen(outputPath);
            strncpy(outputFilePath, outputPath, n);
            outputFilePath[n] = '\0';
        }
    }

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    return PPKG_OK;
}

static int ppkg_depends_output_d2(const char * packageName, const char * outputPath, const char * d2ScriptStr, const size_t d2ScriptStrLength) {
    if (outputPath == NULL) {
        printf("%s\n", d2ScriptStr);
        return PPKG_OK;
    } else {
        char   sessionDIR[PATH_MAX];
        size_t sessionDIRLength;

        int ret = ppkg_session_dir(sessionDIR, &sessionDIRLength);

        if (ret != PPKG_OK) {
            return ret;
        }

        ////////////////////////////////////////////////////////////////

        size_t tmpFilePathCapacity = sessionDIRLength + 18U;
        char   tmpFilePath[tmpFilePathCapacity];

        ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/dependencies.d2", sessionDIR);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        const int fd = open(tmpFilePath, O_CREAT | O_WRONLY | O_TRUNC, 0666);

        if (fd == -1) {
            perror(tmpFilePath);
            return PPKG_ERROR;
        }

        ret = write(fd, d2ScriptStr, d2ScriptStrLength);

        if (ret == -1) {
            perror(tmpFilePath);
            close(fd);
            return PPKG_ERROR;
        }

        close(fd);

        if ((size_t)ret != d2ScriptStrLength) {
            fprintf(stderr, "not fully written: %s\n", tmpFilePath);
            return PPKG_ERROR;
        }

        ////////////////////////////////////////////////////////////////

        char outputFilePath[PATH_MAX];

        ret = get_output_file_path(outputFilePath, packageName, PPKGDependsOutputType_D2, outputPath);

        if (ret != PPKG_OK) {
            return ret;
        }

        return ppkg_rename_or_copy_file(tmpFilePath, outputFilePath);
    }
}


static int ppkg_depends_output_dot(const char * packageName, const char * outputPath, const char * dotScriptStr, const size_t dotScriptStrLength) {
    if (outputPath == NULL) {
        printf("%s\n", dotScriptStr);
        return PPKG_OK;
    } else {
        char   sessionDIR[PATH_MAX];
        size_t sessionDIRLength;

        int ret = ppkg_session_dir(sessionDIR, &sessionDIRLength);

        if (ret != PPKG_OK) {
            return ret;
        }

        ////////////////////////////////////////////////////////////////

        size_t tmpFilePathCapacity = sessionDIRLength + 18U;
        char   tmpFilePath[tmpFilePathCapacity];

        ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/dependencies.dot", sessionDIR);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        int fd = open(tmpFilePath, O_CREAT | O_WRONLY | O_TRUNC, 0666);

        if (fd == -1) {
            perror(tmpFilePath);
            return PPKG_ERROR;
        }

        ret = write(fd, dotScriptStr, dotScriptStrLength);

        if (ret == -1) {
            perror(tmpFilePath);
            close(fd);
            return PPKG_ERROR;
        }

        close(fd);

        if ((size_t)ret != dotScriptStrLength) {
            fprintf(stderr, "not fully written: %s\n", tmpFilePath);
            return PPKG_ERROR;
        }

        ////////////////////////////////////////////////////////////////

        char outputFilePath[PATH_MAX];

        ret = get_output_file_path(outputFilePath, packageName, PPKGDependsOutputType_DOT, outputPath);

        if (ret != PPKG_OK) {
            return ret;
        }

        return ppkg_rename_or_copy_file(tmpFilePath, outputFilePath);
    }
}

static int ppkg_depends_output_box(const char * packageName, const char * outputPath, const char * dotScriptStr, const size_t dotScriptStrLength) {
    size_t urlEncodedBufLength = 3 * dotScriptStrLength + 1U;
    char   urlEncodedBuf[urlEncodedBufLength];
    memset(urlEncodedBuf, 0, urlEncodedBufLength);

    size_t urlEncodedRealLength = 0;

    if (url_encode(urlEncodedBuf, &urlEncodedRealLength, (unsigned char *)dotScriptStr, dotScriptStrLength, true) != 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    size_t urlCapacity = urlEncodedRealLength + 66U;
    char   url[urlCapacity];

    int ret = snprintf(url, urlCapacity, "https://dot-to-ascii.ggerganov.com/dot-to-ascii.php?boxart=1&src=%s", urlEncodedBuf);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    //printf("url=%s\n", url);

    if (outputPath == NULL) {
        int ret = http_fetch_to_stream(url, stdout, false, false);

        if (ret == -1) {
            return PPKG_ERROR;
        }

        if (ret > 0) {
            return PPKG_ERROR_NETWORK_BASE + ret;
        }

        return PPKG_OK;
    } else {
        char   sessionDIR[PATH_MAX];
        size_t sessionDIRLength;

        int ret = ppkg_session_dir(sessionDIR, &sessionDIRLength);

        if (ret != PPKG_OK) {
            return ret;
        }

        ////////////////////////////////////////////////////////////////

        size_t tmpFilePathLength = sessionDIRLength + 18U;
        char   tmpFilePath[tmpFilePathLength];

        ret = snprintf(tmpFilePath, tmpFilePathLength, "%s/dependencies.box", sessionDIR);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        ret = http_fetch_to_file(url, tmpFilePath, false, false);

        if (ret == -1) {
            perror(tmpFilePath);
            return PPKG_ERROR;
        }

        if (ret > 0) {
            return PPKG_ERROR_NETWORK_BASE + ret;
        } else {
            char outputFilePath[PATH_MAX];

            ret = get_output_file_path(outputFilePath, packageName, PPKGDependsOutputType_BOX, outputPath);

            if (ret != PPKG_OK) {
                return ret;
            }

            return ppkg_rename_or_copy_file(tmpFilePath, outputFilePath);
        }
    }
}

static int ppkg_depends_output_via_dot(const char * packageName, PPKGDependsOutputType outputType, const char * outputPath, const char * dotScriptStr, const size_t dotScriptStrLength) {
    char   sessionDIR[PATH_MAX];
    size_t sessionDIRLength;

    int ret = ppkg_session_dir(sessionDIR, &sessionDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    size_t dotFilePathCapacity = sessionDIRLength + 18U;
    char   dotFilePath[dotFilePathCapacity];

    ret = snprintf(dotFilePath, dotFilePathCapacity, "%s/dependencies.dot", sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    int fd = open(dotFilePath, O_CREAT | O_WRONLY | O_TRUNC, 0666);

    if (fd == -1) {
        perror(dotFilePath);
        return PPKG_ERROR;
    }

    ret = write(fd, dotScriptStr, dotScriptStrLength);

    if (ret == -1) {
        perror(dotFilePath);
        close(fd);
        return PPKG_ERROR;
    }

    close(fd);

    if ((size_t)ret != dotScriptStrLength) {
        fprintf(stderr, "not fully written: %s\n", dotFilePath);
        return PPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////

    size_t tmpFilePathCapacity = sessionDIRLength + 18U;
    char   tmpFilePath[tmpFilePathCapacity];

    ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/dependencies.tmp", sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////

    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    ret = ppkg_home_dir(ppkgHomeDIR, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    const char * const str = "/uppm/installed/dot_static/bin/dot_static";

    size_t dotCommandPathCapacity = ppkgHomeDIRLength + strlen(str) + sizeof(char);
    char   dotCommandPath[dotCommandPathCapacity];

    ret = snprintf(dotCommandPath, dotCommandPathCapacity, "%s%s", ppkgHomeDIR, str);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////

    const char * type = (outputType == PPKGDependsOutputType_PNG) ? "-Tpng" : "-Tsvg";

    ////////////////////////////////////////////////////////////////

    int pid = fork();

    if (pid == -1) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (pid == 0) {
        execlp(dotCommandPath, dotCommandPath, type, "-o", tmpFilePath, dotFilePath, NULL);
        perror(dotCommandPath);
        exit(255);
    } else {
        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (childProcessExitStatusCode == 0) {
            char outputFilePath[PATH_MAX];

            ret = get_output_file_path(outputFilePath, packageName, outputType, outputPath);

            if (ret != PPKG_OK) {
                return ret;
            }

            return ppkg_rename_or_copy_file(tmpFilePath, outputFilePath);
        }

        if (WIFEXITED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '%s %s -o %s %s' exit with status code: %d\n", dotCommandPath, type, tmpFilePath, dotFilePath, WEXITSTATUS(childProcessExitStatusCode));
        } else if (WIFSIGNALED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '%s %s -o %s %s' killed by signal: %d\n", dotCommandPath, type, tmpFilePath, dotFilePath, WTERMSIG(childProcessExitStatusCode));
        } else if (WIFSTOPPED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '%s %s -o %s %s' stopped by signal: %d\n", dotCommandPath, type, tmpFilePath, dotFilePath, WSTOPSIG(childProcessExitStatusCode));
        }

        return PPKG_ERROR;
    }
}

static int ppkg_depends_output_via_d2(const char * packageName, PPKGDependsOutputType outputType, const char * outputPath, const char * d2ScriptStr, const size_t d2ScriptStrLength) {
    char   sessionDIR[PATH_MAX];
    size_t sessionDIRLength;

    int ret = ppkg_session_dir(sessionDIR, &sessionDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    size_t d2FilePathCapacity = sessionDIRLength + 18U;
    char   d2FilePath[d2FilePathCapacity];

    ret = snprintf(d2FilePath, d2FilePathCapacity, "%s/dependencies.d2", sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    int fd = open(d2FilePath, O_CREAT | O_WRONLY | O_TRUNC, 0666);

    if (fd == -1) {
        perror(d2FilePath);
        return PPKG_ERROR;
    }

    ret = write(fd, d2ScriptStr, d2ScriptStrLength);

    if (ret == -1) {
        perror(d2FilePath);
        close(fd);
        return PPKG_ERROR;
    }

    close(fd);

    if ((size_t)ret != d2ScriptStrLength) {
        fprintf(stderr, "not fully written: %s\n", d2FilePath);
        return PPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////

    size_t tmpFilePathCapacity = sessionDIRLength + 18U;
    char   tmpFilePath[tmpFilePathCapacity];

    ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/dependencies.%s", sessionDIR, (outputType == PPKGDependsOutputType_PNG) ? "png" : "svg");

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////

    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    ret = ppkg_home_dir(ppkgHomeDIR, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    const char * const str = "/uppm/installed/d2/bin/d2";

    size_t d2CommandPathCapacity = ppkgHomeDIRLength + strlen(str) + sizeof(char);
    char   d2CommandPath[d2CommandPathCapacity];

    ret = snprintf(d2CommandPath, d2CommandPathCapacity, "%s%s", ppkgHomeDIR, str);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////

    int pid = fork();

    if (pid == -1) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (pid == 0) {
        execlp(d2CommandPath, d2CommandPath, d2FilePath, tmpFilePath, NULL);
        perror(d2CommandPath);
        exit(255);
    } else {
        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (childProcessExitStatusCode == 0) {
            char outputFilePath[PATH_MAX];

            ret = get_output_file_path(outputFilePath, packageName, outputType, outputPath);

            if (ret != PPKG_OK) {
                return ret;
            }

            return ppkg_rename_or_copy_file(tmpFilePath, outputFilePath);
        }

        if (WIFEXITED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '%s %s %s' exit with status code: %d\n", d2CommandPath, d2FilePath, tmpFilePath, WEXITSTATUS(childProcessExitStatusCode));
        } else if (WIFSIGNALED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '%s %s %s' killed by signal: %d\n", d2CommandPath, d2FilePath, tmpFilePath, WTERMSIG(childProcessExitStatusCode));
        } else if (WIFSTOPPED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '%s %s %s' stopped by signal: %d\n", d2CommandPath, d2FilePath, tmpFilePath, WSTOPSIG(childProcessExitStatusCode));
        }

        return PPKG_ERROR;
    }
}

int ppkg_depends(const char * packageName, const char * targetPlatformName, PPKGDependsOutputType outputType, const char * outputPath, PPKGDependsOutputDiagramEngine engine) {
    bool needGenerateDotScript;

    switch (outputType) {
        case PPKGDependsOutputType_D2 : needGenerateDotScript = false;
        case PPKGDependsOutputType_DOT: needGenerateDotScript = true;
        case PPKGDependsOutputType_BOX: needGenerateDotScript = true;
        default:
            if (engine == PPKGDependsOutputDiagramEngine_DOT) {
                needGenerateDotScript = true;
            } else {
                needGenerateDotScript = false;
            }
    }

    ////////////////////////////////////////////////////////////////

    int ret = PPKG_OK;

    ////////////////////////////////////////////////////////////////

    char * p = NULL;
    size_t pSize = 0;
    size_t pCapacity = 0;

    ////////////////////////////////////////////////////////////////

    size_t  packageNameArrayCapacity = 0;
    size_t  packageNameArraySize     = 0;
    char ** packageNameArray         = NULL;

    ////////////////////////////////////////////////////////////////

    size_t  packageNameStackCapacity = 1;
    size_t  packageNameStackSize     = 1;
    char ** packageNameStack         = (char**)calloc(1, sizeof(char*));

    if (packageNameStack == NULL) {
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    packageNameStack[0] = strdup(packageName);

    if (packageNameStack[0] == NULL) {
        free(packageNameStack);
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    ////////////////////////////////////////////////////////////////

    while (packageNameStackSize > 0) {
        char * packageName = packageNameStack[--packageNameStackSize];
        packageNameStack[packageNameStackSize] = NULL;

        ////////////////////////////////////////////////////////////////

        bool hasHandled = false;

        for (size_t i = 0; i < packageNameArraySize; i++) {
            if (strcmp(packageNameArray[i], packageName) == 0) {
                free(packageName);
                hasHandled = true;
                break;
            }
        }

        if (hasHandled) continue;

        ////////////////////////////////////////////////////////////////

        if (packageNameArraySize == packageNameArrayCapacity) {
            char ** p = (char**)realloc(packageNameArray, (packageNameArrayCapacity + 10U) * sizeof(char*));

            if (p == NULL) {
                free(packageName);
                ret = PPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            memset(p + packageNameArrayCapacity, 0, 10);

            packageNameArray = p;
            packageNameArrayCapacity += 10;
        }

        packageNameArray[packageNameArraySize++] = packageName;

        ////////////////////////////////////////////////////////////////

        PPKGFormula * formula = NULL;

        ret = ppkg_formula_load(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            goto finalize;
        }

        if (formula->dep_pkg == NULL) {
            ppkg_formula_free(formula);
            continue;
        }

        ////////////////////////////////////////////////////////////////

        size_t packageNameLength = strlen(packageName);

        if (needGenerateDotScript) {
            size_t bufCapacity = packageNameLength + 12U;
            char   buf[bufCapacity];

            ret = snprintf(buf, bufCapacity, "    \"%s\" -> {", packageName);

            if (ret < 0) {
                perror(NULL);
                ppkg_formula_free(formula);
                return PPKG_ERROR;
            }

            ret = string_append(&p, &pSize, &pCapacity, buf, ret);

            if (ret != PPKG_OK) {
                ppkg_formula_free(formula);
                goto finalize;
            }
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
                ppkg_formula_free(formula);
                ret = PPKG_ERROR;
                goto finalize;
            }

            ////////////////////////////////////////////////////////////////

            if (needGenerateDotScript) {
                size_t bufLength = strlen(depPackageName) + 4U;
                char   buf[bufLength];

                ret = snprintf(buf, bufLength, " \"%s\"", depPackageName);

                if (ret < 0) {
                    perror(NULL);
                    ppkg_formula_free(formula);
                    return PPKG_ERROR;
                }

                ret = string_append(&p, &pSize, &pCapacity, buf, ret);

                if (ret != PPKG_OK) {
                    ppkg_formula_free(formula);
                    goto finalize;
                }
            } else {
                size_t bufCapacity = packageNameLength + strlen(depPackageName) + 10U;
                char   buf[bufCapacity];

                ret = snprintf(buf, bufCapacity, "\"%s\" -> \"%s\"\n", packageName, depPackageName);

                if (ret < 0) {
                    perror(NULL);
                    ppkg_formula_free(formula);
                    return PPKG_ERROR;
                }

                ret = string_append(&p, &pSize, &pCapacity, buf, ret);

                if (ret != PPKG_OK) {
                    ppkg_formula_free(formula);
                    goto finalize;
                }
            }

            ////////////////////////////////////////////////////////////////

            if (packageNameStackSize == packageNameStackCapacity) {
                char ** p = (char**)realloc(packageNameStack, (packageNameStackCapacity + 10U) * sizeof(char*));

                if (p == NULL) {
                    ppkg_formula_free(formula);
                    ret = PPKG_ERROR_MEMORY_ALLOCATE;
                    goto finalize;
                }

                memset(p + packageNameStackCapacity, 0, 10);

                packageNameStack = p;
                packageNameStackCapacity += 10;
            }

            char * p = strdup(depPackageName);

            if (p == NULL) {
                ppkg_formula_free(formula);
                ret = PPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            packageNameStack[packageNameStackSize] = p;
            packageNameStackSize++;

            depPackageName = strtok (NULL, " ");
        }

        if (needGenerateDotScript) {
            ret = string_append(&p, &pSize, &pCapacity, " }\n", 3);
        }

        ppkg_formula_free(formula);
    }

finalize:
    for (size_t i = 0U; i < packageNameStackSize; i++) {
        free(packageNameStack[i]);
        packageNameStack[i] = NULL;
    }

    free(packageNameStack);
    packageNameStack = NULL;

    //////////////////////////////////////////////////

    for (size_t i = 0U; i < packageNameArraySize; i++) {
        free(packageNameArray[i]);
        packageNameArray[i] = NULL;
    }

    free(packageNameArray);
    packageNameArray = NULL;

    //////////////////////////////////////////////////

    if (ret != PPKG_OK) {
        free(p);
        return ret;
    }

    if (pSize == 0U) {
        return PPKG_OK;
    }

    //////////////////////////////////////////////////

    size_t stringBufferLength;
    size_t stringBufferCapacity = pSize + 14U;
    char   stringBuffer[stringBufferCapacity];

    if (needGenerateDotScript) {
        int ret = snprintf(stringBuffer, stringBufferCapacity, "digraph G {\n%s}", p);

        if (ret < 0) {
            perror(NULL);
            free(p);
            return PPKG_ERROR;
        }

        stringBufferLength = ret;
    } else {
        strncpy(stringBuffer, p, pSize);
        stringBuffer[pSize] = '\0';
        stringBufferLength = pSize;
    }

    free(p);

    //////////////////////////////////////////////////

    switch (outputType) {
        case PPKGDependsOutputType_D2 : return ppkg_depends_output_d2 (packageName, outputPath, stringBuffer, stringBufferLength);
        case PPKGDependsOutputType_DOT: return ppkg_depends_output_dot(packageName, outputPath, stringBuffer, stringBufferLength);
        case PPKGDependsOutputType_BOX: return ppkg_depends_output_box(packageName, outputPath, stringBuffer, stringBufferLength);
        default:
            if (engine == PPKGDependsOutputDiagramEngine_DOT) {
                return ppkg_depends_output_via_dot(packageName, outputType, outputPath, stringBuffer, stringBufferLength);
            } else {
                return ppkg_depends_output_via_d2 (packageName, outputType, outputPath, stringBuffer, stringBufferLength);
            }
    }
}
