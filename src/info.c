#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <jansson.h>

#include "core/log.h"
#include "ppkg.h"

static int package_name_callback(const char * packageName, size_t i, const void * key) {
    if (i != 0) {
        printf("\n");
    }

    return ppkg_info(packageName, (char*)key);
}

int ppkg_info(const char * packageName, const char * key) {
    if (packageName == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (packageName[0] == '\0') {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    if (strcmp(packageName, "@all") == 0) {
        return ppkg_list_the_available_packages(package_name_callback, key);
    }

    int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    if ((key == NULL) || (key[0] == '\0') || (strcmp(key, "formula-yaml") == 0)) {
        char * formulaFilePath = NULL;

        ret = ppkg_formula_locate(packageName, &formulaFilePath);

        if (ret != PPKG_OK) {
            return ret;
        }

        FILE * formulaFile = fopen(formulaFilePath, "r");

        if (formulaFile == NULL) {
            perror(formulaFilePath);
            free(formulaFilePath);
            return PPKG_ERROR;
        }

        if (isatty(STDOUT_FILENO)) {
            if (ppkg_check_if_the_given_package_is_installed(packageName) == PPKG_OK) {
                printf("pkgname: %s%s%s\n", COLOR_GREEN, packageName, COLOR_OFF);
            } else {
                printf("pkgname: %s%s%s\n", COLOR_RED,   packageName, COLOR_OFF);
            }
        } else {
            printf("pkgname: %s\n", packageName);
        }

        char   buff[1024];
        size_t size;

        for (;;) {
            size = fread(buff, 1, 1024, formulaFile);

            if (ferror(formulaFile)) {
                perror(formulaFilePath);
                fclose(formulaFile);
                free(formulaFilePath);
                return PPKG_ERROR;
            }

            if (size > 0) {
                if (fwrite(buff, 1, size, stdout) != size || ferror(stdout)) {
                    perror(NULL);
                    fclose(formulaFile);
                    free(formulaFilePath);
                    return PPKG_ERROR;
                }
            }

            if (feof(formulaFile)) {
                fclose(formulaFile);
                break;
            }
        }

        printf("formula: %s\n", formulaFilePath);

        free(formulaFilePath);
    } else if (strcmp(key, "formula-json") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        json_t * root = json_object();

        json_object_set_new(root, "pkgname", json_string(packageName));

        json_object_set_new(root, "summary", json_string(formula->summary));
        json_object_set_new(root, "version", json_string(formula->version));
        json_object_set_new(root, "license", json_string(formula->license));

        json_object_set_new(root, "web-url", json_string(formula->web_url));

        json_object_set_new(root, "git-url", json_string(formula->git_url));
        json_object_set_new(root, "git-sha", json_string(formula->git_sha));
        json_object_set_new(root, "git-ref", json_string(formula->git_ref));

        json_object_set_new(root, "src-url", json_string(formula->src_url));
        json_object_set_new(root, "src-sha", json_string(formula->src_sha));

        json_object_set_new(root, "fix-url", json_string(formula->fix_url));
        json_object_set_new(root, "fix-sha", json_string(formula->fix_sha));

        json_object_set_new(root, "res-url", json_string(formula->res_url));
        json_object_set_new(root, "res-sha", json_string(formula->res_sha));

        json_object_set_new(root, "dep-pkg", json_string(formula->dep_pkg));
        json_object_set_new(root, "dep-upp", json_string(formula->dep_upp));
        json_object_set_new(root, "dep-pym", json_string(formula->dep_pym));
        json_object_set_new(root, "dep-plm", json_string(formula->dep_plm));

        json_object_set_new(root, "ppflags", json_string(formula->ppflags));
        json_object_set_new(root, "ccflags", json_string(formula->ccflags));
        json_object_set_new(root, "xxflags", json_string(formula->xxflags));
        json_object_set_new(root, "ldfalgs", json_string(formula->ldflags));

        json_object_set_new(root, "bsystem", json_string(formula->bscript));
        json_object_set_new(root, "bscript", json_string(formula->bscript));
        json_object_set_new(root, "binbstd", json_boolean(formula->binbstd));
        json_object_set_new(root, "parallel", json_boolean(formula->parallel));
        json_object_set_new(root, "symlink", json_boolean(formula->symlink));

        json_object_set_new(root, "exetype", json_string(formula->exetype));

        json_object_set_new(root, "dopatch", json_string(formula->dopatch));
        json_object_set_new(root, "install", json_string(formula->install));

        char * jsonStr = json_dumps(root, 0);

        if (jsonStr == NULL) {
            ret = PPKG_ERROR;
        } else {
            printf("%s\n", jsonStr);
            free(jsonStr);
        }

        json_decref(root);

        ppkg_formula_free(formula);
    } else if (strcmp(key, "formula-path") == 0) {
        char * formulaFilePath = NULL;

        ret = ppkg_formula_locate(packageName, &formulaFilePath);

        if (ret != PPKG_OK) {
            return ret;
        }

        printf("%s\n", formulaFilePath);
        free(formulaFilePath);
    } else if (strcmp(key, "summary") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->summary != NULL) {
            printf("%s\n", formula->summary);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "version") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->version != NULL) {
            printf("%s\n", formula->version);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "license") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->license != NULL) {
            printf("%s\n", formula->license);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "web-url") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->web_url != NULL) {
            printf("%s\n", formula->web_url);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "git-url") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->git_url != NULL) {
            printf("%s\n", formula->git_url);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "git-sha") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->git_sha != NULL) {
            printf("%s\n", formula->git_sha);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "git-ref") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->git_ref != NULL) {
            printf("%s\n", formula->git_ref);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "src-url") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->src_url != NULL) {
            printf("%s\n", formula->src_url);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "src-sha") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->src_sha != NULL) {
            printf("%s\n", formula->src_sha);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "fix-url") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->fix_url != NULL) {
            printf("%s\n", formula->fix_url);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "fix-sha") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->fix_sha != NULL) {
            printf("%s\n", formula->fix_sha);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "res-url") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->res_url != NULL) {
            printf("%s\n", formula->res_url);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "res-sha") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->res_sha != NULL) {
            printf("%s\n", formula->res_sha);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "dep-pkg") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->dep_pkg != NULL) {
            printf("%s\n", formula->dep_pkg);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "dep-upp") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->dep_upp != NULL) {
            printf("%s\n", formula->dep_upp);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "dep-pym") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->dep_pym != NULL) {
            printf("%s\n", formula->dep_pym);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "dep-plm") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->dep_plm != NULL) {
            printf("%s\n", formula->dep_plm);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "bsystem") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->bsystem != NULL) {
            printf("%s\n", formula->bsystem);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "bscript") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->bscript != NULL) {
            printf("%s\n", formula->bscript);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "binbstd") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        printf("%s\n", formula->binbstd ? "yes" : "no");

        ppkg_formula_free(formula);
    } else if (strcmp(key, "symlink") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        printf("%s\n", formula->symlink ? "yes" : "no");

        ppkg_formula_free(formula);
    } else if (strcmp(key, "parallel") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        printf("%s\n", formula->parallel ? "yes" : "no");

        ppkg_formula_free(formula);
    } else if (strcmp(key, "ppflags") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->ppflags != NULL) {
            printf("%s\n", formula->ppflags);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "ccflags") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->ccflags != NULL) {
            printf("%s\n", formula->ccflags);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "xxflags") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->xxflags != NULL) {
            printf("%s\n", formula->xxflags);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "ldflags") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->ldflags != NULL) {
            printf("%s\n", formula->ldflags);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "exetype") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->exetype != NULL) {
            printf("%s\n", formula->exetype);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "dopatch") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->dopatch != NULL) {
            printf("%s\n", formula->dopatch);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "install") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->install != NULL) {
            printf("%s\n", formula->install);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "installed-dir") == 0) {
        char * userHomeDir = getenv("HOME");

        if (userHomeDir == NULL || userHomeDir[0] == '\0') {
            return PPKG_ERROR_ENV_HOME_NOT_SET;
        }

        size_t userHomeDirLength = strlen(userHomeDir);

        size_t  installedDirLength = userHomeDirLength + strlen(packageName) + 20U;
        char    installedDir[installedDirLength];
        snprintf(installedDir, installedDirLength, "%s/.ppkg/installed/%s", userHomeDir, packageName);

        size_t  receiptFilePathLength = installedDirLength + 20U;
        char    receiptFilePath[receiptFilePathLength];
        snprintf(receiptFilePath, receiptFilePathLength, "%s/.ppkg/receipt.yml", installedDir);

        struct stat st;

        if (stat(receiptFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            printf("%s\n", installedDir);
        } else {
            return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
        }
    } else if (strcmp(key, "installed-files") == 0) {
        char * userHomeDir = getenv("HOME");

        if (userHomeDir == NULL || userHomeDir[0] == '\0') {
            return PPKG_ERROR_ENV_HOME_NOT_SET;
        }

        size_t userHomeDirLength = strlen(userHomeDir);

        size_t  installedDirLength = userHomeDirLength + strlen(packageName) + 20U;
        char    installedDir[installedDirLength];
        snprintf(installedDir, installedDirLength, "%s/.ppkg/installed/%s", userHomeDir, packageName);

        size_t  receiptFilePathLength = installedDirLength + 20U;
        char    receiptFilePath[receiptFilePathLength];
        snprintf(receiptFilePath, receiptFilePathLength, "%s/.ppkg/receipt.yml", installedDir);

        struct stat st;

        if (stat(receiptFilePath, &st) != 0 || !S_ISREG(st.st_mode)) {
            return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
        }

        size_t  installedManifestFilePathLength = installedDirLength + 20U;
        char    installedManifestFilePath[installedManifestFilePathLength];
        snprintf(installedManifestFilePath, installedManifestFilePathLength, "%s/.ppkg/manifest.txt", installedDir);

        FILE * installedManifestFile = fopen(installedManifestFilePath, "r");

        if (installedManifestFile == NULL) {
            perror(installedManifestFilePath);
            return PPKG_ERROR;
        }

        char   buff[1024];
        size_t size;

        for (;;) {
            size = fread(buff, 1, 1024, installedManifestFile);

            if (ferror(installedManifestFile)) {
                perror(installedManifestFilePath);
                fclose(installedManifestFile);
                return PPKG_ERROR;
            }

            if (size > 0) {
                if (fwrite(buff, 1, size, stdout) != size || ferror(stdout)) {
                    perror(NULL);
                    fclose(installedManifestFile);
                    return PPKG_ERROR;
                }
            }

            if (feof(installedManifestFile)) {
                fclose(installedManifestFile);
                break;
            }
        }
    } else if (strcmp(key, "installed-version") == 0) {
        PPKGReceipt * receipt = NULL;

        int ret = ppkg_receipt_parse(packageName, &receipt);

        if (ret != PPKG_OK) {
            return ret;
        }

        printf("%s\n", receipt->version);

        ppkg_receipt_free(receipt);
    } else if (strcmp(key, "installed-timestamp-unix") == 0) {
        PPKGReceipt * receipt = NULL;

        int ret = ppkg_receipt_parse(packageName, &receipt);

        if (ret != PPKG_OK) {
            return ret;
        }

        printf("%s\n", receipt->timestamp);

        ppkg_receipt_free(receipt);
    } else if (strcmp(key, "installed-timestamp-rfc-3339") == 0) {
        PPKGReceipt * receipt = NULL;

        int ret = ppkg_receipt_parse(packageName, &receipt);

        if (ret != PPKG_OK) {
            return ret;
        }

        time_t tt = (time_t)atol(receipt->timestamp);
        struct tm *tms = localtime(&tt);

        char buff[26] = {0};
        strftime(buff, 26, "%Y-%m-%d %H:%M:%S%z", tms);

        buff[24] = buff[23];
        buff[23] = buff[22];
        buff[22] = ':';

        printf("%s\n", buff);

        ppkg_receipt_free(receipt);
    } else if (strcmp(key, "installed-timestamp-rfc-3339-utc") == 0) {
        PPKGReceipt * receipt = NULL;

        int ret = ppkg_receipt_parse(packageName, &receipt);

        if (ret != PPKG_OK) {
            return ret;
        }

        time_t tt = (time_t)atol(receipt->timestamp);
        struct tm *tms = gmtime(&tt);

        char buff[26] = {0};
        strftime(buff, 26, "%Y-%m-%d %H:%M:%S%z", tms);

        buff[24] = buff[23];
        buff[23] = buff[22];
        buff[22] = ':';

        printf("%s\n", buff);

        ppkg_receipt_free(receipt);
    } else if (strcmp(key, "installed-timestamp-iso-8601") == 0) {
        PPKGReceipt * receipt = NULL;

        int ret = ppkg_receipt_parse(packageName, &receipt);

        if (ret != PPKG_OK) {
            return ret;
        }

        time_t tt = (time_t)atol(receipt->timestamp);
        struct tm *tms = localtime(&tt);

        char buff[26] = {0};
        strftime(buff, 26, "%Y-%m-%dT%H:%M:%S%z", tms);

        buff[24] = buff[23];
        buff[23] = buff[22];
        buff[22] = ':';

        printf("%s\n", buff);

        ppkg_receipt_free(receipt);
    } else if (strcmp(key, "installed-timestamp-iso-8601-utc") == 0) {
        PPKGReceipt * receipt = NULL;

        int ret = ppkg_receipt_parse(packageName, &receipt);

        if (ret != PPKG_OK) {
            return ret;
        }

        time_t tt = (time_t)atol(receipt->timestamp);
        struct tm *tms = gmtime(&tt);

        char buff[21] = {0};
        strftime(buff, 21, "%Y-%m-%dT%H:%M:%SZ", tms);

        printf("%s\n", buff);

        ppkg_receipt_free(receipt);
    } else if (strcmp(key, "receipt-path") == 0) {
        char * userHomeDir = getenv("HOME");

        if (userHomeDir == NULL || userHomeDir[0] == '\0') {
            return PPKG_ERROR_ENV_HOME_NOT_SET;
        }

        size_t userHomeDirLength = strlen(userHomeDir);

        size_t  installedDirLength = userHomeDirLength + strlen(packageName) + 20U;
        char    installedDir[installedDirLength];
        snprintf(installedDir, installedDirLength, "%s/.ppkg/installed/%s", userHomeDir, packageName);

        size_t  receiptFilePathLength = installedDirLength + 20U;
        char    receiptFilePath[receiptFilePathLength];
        snprintf(receiptFilePath, receiptFilePathLength, "%s/.ppkg/receipt.yml", installedDir);

        struct stat st;

        if (stat(receiptFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            printf("%s\n", receiptFilePath);
        } else {
            return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
        }
    } else if (strcmp(key, "receipt-yaml") == 0) {
        char * userHomeDir = getenv("HOME");

        if (userHomeDir == NULL || userHomeDir[0] == '\0') {
            return PPKG_ERROR_ENV_HOME_NOT_SET;
        }

        size_t userHomeDirLength = strlen(userHomeDir);

        size_t  installedDirLength = userHomeDirLength + strlen(packageName) + 20U;
        char    installedDir[installedDirLength];
        snprintf(installedDir, installedDirLength, "%s/.ppkg/installed/%s", userHomeDir, packageName);

        size_t  receiptFilePathLength = installedDirLength + 20U;
        char    receiptFilePath[receiptFilePathLength];
        snprintf(receiptFilePath, receiptFilePathLength, "%s/.ppkg/receipt.yml", installedDir);

        struct stat st;

        if (stat(receiptFilePath, &st) != 0 || !S_ISREG(st.st_mode)) {
            return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
        }

        FILE * receiptFile = fopen(receiptFilePath, "r");

        if (receiptFile == NULL) {
            perror(receiptFilePath);
            return PPKG_ERROR;
        }

        char   buff[1024];
        size_t size;

        for (;;) {
            size = fread(buff, 1, 1024, receiptFile);

            if (ferror(receiptFile)) {
                perror(receiptFilePath);
                fclose(receiptFile);
                return PPKG_ERROR;
            }

            if (size > 0) {
                if (fwrite(buff, 1, size, stdout) != size || ferror(stdout)) {
                    perror(NULL);
                    fclose(receiptFile);
                    return PPKG_ERROR;
                }
            }

            if (feof(receiptFile)) {
                fclose(receiptFile);
                break;
            }
        }
    } else if (strcmp(key, "receipt-json") == 0) {
        PPKGReceipt * receipt = NULL;

        int ret = ppkg_receipt_parse(packageName, &receipt);

        if (ret != PPKG_OK) {
            return ret;
        }

        json_t * root = json_object();

        json_object_set_new(root, "pkgname", json_string(packageName));

        json_object_set_new(root, "summary", json_string(receipt->summary));
        json_object_set_new(root, "version", json_string(receipt->version));
        json_object_set_new(root, "license", json_string(receipt->license));

        json_object_set_new(root, "web-url", json_string(receipt->web_url));

        json_object_set_new(root, "git-url", json_string(receipt->git_url));
        json_object_set_new(root, "git-sha", json_string(receipt->git_sha));
        json_object_set_new(root, "git-ref", json_string(receipt->git_ref));

        json_object_set_new(root, "src-url", json_string(receipt->src_url));
        json_object_set_new(root, "src-sha", json_string(receipt->src_sha));

        json_object_set_new(root, "fix-url", json_string(receipt->fix_url));
        json_object_set_new(root, "fix-sha", json_string(receipt->fix_sha));

        json_object_set_new(root, "res-url", json_string(receipt->res_url));
        json_object_set_new(root, "res-sha", json_string(receipt->res_sha));

        json_object_set_new(root, "dep-pkg", json_string(receipt->dep_pkg));
        json_object_set_new(root, "dep-upp", json_string(receipt->dep_upp));
        json_object_set_new(root, "dep-pym", json_string(receipt->dep_pym));
        json_object_set_new(root, "dep-plm", json_string(receipt->dep_plm));

        json_object_set_new(root, "ppflags", json_string(receipt->ppflags));
        json_object_set_new(root, "ccflags", json_string(receipt->ccflags));
        json_object_set_new(root, "xxflags", json_string(receipt->xxflags));
        json_object_set_new(root, "ldfalgs", json_string(receipt->ldflags));

        json_object_set_new(root, "bsystem", json_string(receipt->bscript));
        json_object_set_new(root, "bscript", json_string(receipt->bscript));
        json_object_set_new(root, "binbstd", json_boolean(receipt->binbstd));
        json_object_set_new(root, "parallel", json_boolean(receipt->parallel));
        json_object_set_new(root, "symlink", json_boolean(receipt->symlink));

        json_object_set_new(root, "exetype", json_string(receipt->exetype));

        json_object_set_new(root, "dopatch", json_string(receipt->dopatch));
        json_object_set_new(root, "install", json_string(receipt->install));

        json_object_set_new(root, "signature", json_string(receipt->signature));
        json_object_set_new(root, "timestamp", json_string(receipt->timestamp));

        char * jsonStr = json_dumps(root, 0);

        if (jsonStr == NULL) {
            ret = PPKG_ERROR;
        } else {
            printf("%s\n", jsonStr);
            free(jsonStr);
        }

        json_decref(root);

        ppkg_receipt_free(receipt);
    } else {
        return PPKG_ERROR_ARG_IS_UNKNOWN;
    }

    return ret;
}
