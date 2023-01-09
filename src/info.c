#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <jansson.h>

#include "core/fs.h"
#include "core/log.h"
#include "ppkg.h"

#include <dirent.h>
#include <fnmatch.h>

int ppkg_info_all_available_packages(const char * key) {
    PPKGFormulaRepoList * formulaRepoList = NULL;

    int resultCode = ppkg_formula_repo_list_new(&formulaRepoList);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    bool isFirst = true;

    for (size_t i = 0; i < formulaRepoList->size; i++) {
        char *  formulaRepoPath  = formulaRepoList->repos[i]->path;
        size_t  formulaDirLength = strlen(formulaRepoPath) + 10;
        char    formulaDir[formulaDirLength];
        memset (formulaDir, 0, formulaDirLength);
        sprintf(formulaDir, "%s/formula", formulaRepoPath);

        DIR           * dir;
        struct dirent * dir_entry;

        dir = opendir(formulaDir);

        if (dir == NULL) {
            ppkg_formula_repo_list_free(formulaRepoList);
            perror(formulaDir);
            return PPKG_ERROR;
        }

        while ((dir_entry = readdir(dir))) {
            //puts(dir_entry->d_name);
            if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
                continue;
            }

            int r = fnmatch("*.yml", dir_entry->d_name, 0);

            if (r == 0) {
                size_t  fileNameLength = strlen(dir_entry->d_name);
                char    packageName[fileNameLength];
                memset (packageName, 0, fileNameLength);
                strncpy(packageName, dir_entry->d_name, fileNameLength - 4);

                if (!isFirst) {
                    printf("\n");
                }

                //printf("%s\n", packageName);
                resultCode = ppkg_info(packageName, key);

                if (resultCode != PPKG_OK) {
                    ppkg_formula_repo_list_free(formulaRepoList);
                    closedir(dir);
                    return resultCode;
                }

                if (isFirst) {
                    isFirst = false;
                }
            } else if(r == FNM_NOMATCH) {
                ;
            } else {
                ppkg_formula_repo_list_free(formulaRepoList);
                fprintf(stderr, "fnmatch() error\n");
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        closedir(dir);
    }

    ppkg_formula_repo_list_free(formulaRepoList);

    return PPKG_OK;
}

int ppkg_info(const char * packageName, const char * key) {
    if (packageName == NULL) {
        return PPKG_ARG_IS_NULL;
    }

    if (strcmp(packageName, "") == 0) {
        return PPKG_ARG_IS_EMPTY;
    }

    if (strcmp(packageName, "@all") == 0) {
        return ppkg_info_all_available_packages(key);
    }

    int resultCode = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    if ((key == NULL) || (strcmp(key, "") == 0) || (strcmp(key, "formula-yaml") == 0)) {
        char * formulaFilePath = NULL;

        resultCode = ppkg_formula_path(packageName, &formulaFilePath);

        if (resultCode != PPKG_OK) {
            return resultCode;
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
        size_t size = 0;
        while((size = fread(buff, 1, 1024, formulaFile)) != 0) {
            fwrite(buff, 1, size, stdout);
        }

        fclose(formulaFile);

        printf("formula: %s\n", formulaFilePath);

        free(formulaFilePath);
    } else if (strcmp(key, "formula-json") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
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

        json_object_set_new(root, "cdefine", json_string(formula->cdefine));
        json_object_set_new(root, "ccflags", json_string(formula->ccflags));
        json_object_set_new(root, "xxflags", json_string(formula->xxflags));
        json_object_set_new(root, "ldfalgs", json_string(formula->ldflags));

        json_object_set_new(root, "bsystem", json_string(formula->bscript));
        json_object_set_new(root, "bscript", json_string(formula->bscript));
        json_object_set_new(root, "binbstd", json_boolean(formula->binbstd));
        json_object_set_new(root, "parallel", json_boolean(formula->parallel));
        json_object_set_new(root, "symlink", json_boolean(formula->symlink));

        json_object_set_new(root, "exetype", json_string(formula->exetype));

        json_object_set_new(root, "prepare", json_string(formula->prepare));
        json_object_set_new(root, "install", json_string(formula->install));

        char * jsonStr = json_dumps(root, 0);

        if (jsonStr == NULL) {
            resultCode = PPKG_ERROR;
        } else {
            printf("%s\n", jsonStr);
            free(jsonStr);
        }

        json_decref(root);

        ppkg_formula_free(formula);
    } else if (strcmp(key, "formula-path") == 0) {
        char * formulaFilePath = NULL;

        resultCode = ppkg_formula_path(packageName, &formulaFilePath);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        printf("%s\n", formulaFilePath);
        free(formulaFilePath);
    } else if (strcmp(key, "summary") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->summary != NULL) {
            printf("%s\n", formula->summary);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "version") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->version != NULL) {
            printf("%s\n", formula->version);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "license") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->license != NULL) {
            printf("%s\n", formula->license);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "web-url") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->web_url != NULL) {
            printf("%s\n", formula->web_url);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "git-url") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->git_url != NULL) {
            printf("%s\n", formula->git_url);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "git-sha") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->git_sha != NULL) {
            printf("%s\n", formula->git_sha);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "git-ref") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->git_ref != NULL) {
            printf("%s\n", formula->git_ref);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "src-url") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->src_url != NULL) {
            printf("%s\n", formula->src_url);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "src-sha") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->src_sha != NULL) {
            printf("%s\n", formula->src_sha);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "fix-url") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->fix_url != NULL) {
            printf("%s\n", formula->fix_url);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "fix-sha") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->fix_sha != NULL) {
            printf("%s\n", formula->fix_sha);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "res-url") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->res_url != NULL) {
            printf("%s\n", formula->res_url);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "res-sha") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->res_sha != NULL) {
            printf("%s\n", formula->res_sha);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "dep-pkg") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->dep_pkg != NULL) {
            printf("%s\n", formula->dep_pkg);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "dep-upp") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->dep_upp != NULL) {
            printf("%s\n", formula->dep_upp);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "dep-pym") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->dep_pym != NULL) {
            printf("%s\n", formula->dep_pym);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "dep-plm") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->dep_plm != NULL) {
            printf("%s\n", formula->dep_plm);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "bsystem") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->bsystem != NULL) {
            printf("%s\n", formula->bsystem);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "bscript") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->bscript != NULL) {
            printf("%s\n", formula->bscript);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "binbstd") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        printf("%s\n", formula->binbstd ? "yes" : "no");

        ppkg_formula_free(formula);
    } else if (strcmp(key, "symlink") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        printf("%s\n", formula->symlink ? "yes" : "no");

        ppkg_formula_free(formula);
    } else if (strcmp(key, "parallel") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        printf("%s\n", formula->parallel ? "yes" : "no");

        ppkg_formula_free(formula);
    } else if (strcmp(key, "cdefine") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->cdefine != NULL) {
            printf("%s\n", formula->cdefine);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "ccflags") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->ccflags != NULL) {
            printf("%s\n", formula->ccflags);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "xxflags") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->xxflags != NULL) {
            printf("%s\n", formula->xxflags);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "ldflags") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->ldflags != NULL) {
            printf("%s\n", formula->ldflags);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "exetype") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->exetype != NULL) {
            printf("%s\n", formula->exetype);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "prepare") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->prepare != NULL) {
            printf("%s\n", formula->prepare);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "install") == 0) {
        PPKGFormula * formula = NULL;

        resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        if (formula->install != NULL) {
            printf("%s\n", formula->install);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "installed-dir") == 0) {
        char * userHomeDir = getenv("HOME");

        if (userHomeDir == NULL || strcmp(userHomeDir, "") == 0) {
            return PPKG_ENV_HOME_NOT_SET;
        }

        size_t userHomeDirLength = strlen(userHomeDir);

        size_t  installedDirLength = userHomeDirLength + strlen(packageName) + 20;
        char    installedDir[installedDirLength];
        memset (installedDir, 0, installedDirLength);
        sprintf(installedDir, "%s/.ppkg/installed/%s", userHomeDir, packageName);

        size_t  receiptFilePathLength = installedDirLength + 20;
        char    receiptFilePath[receiptFilePathLength];
        memset (receiptFilePath, 0, receiptFilePathLength);
        sprintf(receiptFilePath, "%s/.ppkg/receipt.yml", installedDir);

        if (exists_and_is_a_regular_file(receiptFilePath)) {
            printf("%s\n", installedDir);
        } else {
            return PPKG_PACKAGE_IS_NOT_INSTALLED;
        }
    } else if (strcmp(key, "installed-files") == 0) {
        char * userHomeDir = getenv("HOME");

        if (userHomeDir == NULL || strcmp(userHomeDir, "") == 0) {
            return PPKG_ENV_HOME_NOT_SET;
        }

        size_t userHomeDirLength = strlen(userHomeDir);

        size_t  installedDirLength = userHomeDirLength + strlen(packageName) + 20;
        char    installedDir[installedDirLength];
        memset (installedDir, 0, installedDirLength);
        sprintf(installedDir, "%s/.ppkg/installed/%s", userHomeDir, packageName);

        size_t  receiptFilePathLength = installedDirLength + 20;
        char    receiptFilePath[receiptFilePathLength];
        memset (receiptFilePath, 0, receiptFilePathLength);
        sprintf(receiptFilePath, "%s/.ppkg/receipt.yml", installedDir);

        if (!exists_and_is_a_regular_file(receiptFilePath)) {
            return PPKG_PACKAGE_IS_NOT_INSTALLED;
        }

        size_t  installedManifestFilePathLength = installedDirLength + 20;
        char    installedManifestFilePath[installedManifestFilePathLength];
        memset (installedManifestFilePath, 0, installedManifestFilePathLength);
        sprintf(installedManifestFilePath, "%s/.ppkg/manifest.txt", installedDir);

        FILE * installedManifestFile = fopen(installedManifestFilePath, "r");

        if (installedManifestFile == NULL) {
            perror(installedManifestFilePath);
            return PPKG_ERROR;
        }

        char buff[1024];
        int  size = 0;
        while((size = fread(buff, 1, 1024, installedManifestFile)) != 0) {
            fwrite(buff, 1, size, stdout);
        }

        fclose(installedManifestFile);
    } else if (strcmp(key, "installed-version") == 0) {
        PPKGReceipt * receipt = NULL;

        int resultCode = ppkg_receipt_parse(packageName, &receipt);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        printf("%s\n", receipt->version);

        ppkg_receipt_free(receipt);
    } else if (strcmp(key, "installed-timestamp-unix") == 0) {
        PPKGReceipt * receipt = NULL;

        int resultCode = ppkg_receipt_parse(packageName, &receipt);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        printf("%s\n", receipt->timestamp);

        ppkg_receipt_free(receipt);
    } else if (strcmp(key, "installed-timestamp-rfc-3339") == 0) {
        PPKGReceipt * receipt = NULL;

        int resultCode = ppkg_receipt_parse(packageName, &receipt);

        if (resultCode != PPKG_OK) {
            return resultCode;
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

        int resultCode = ppkg_receipt_parse(packageName, &receipt);

        if (resultCode != PPKG_OK) {
            return resultCode;
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

        int resultCode = ppkg_receipt_parse(packageName, &receipt);

        if (resultCode != PPKG_OK) {
            return resultCode;
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

        int resultCode = ppkg_receipt_parse(packageName, &receipt);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        time_t tt = (time_t)atol(receipt->timestamp);
        struct tm *tms = gmtime(&tt);

        char buff[21] = {0};
        strftime(buff, 21, "%Y-%m-%dT%H:%M:%SZ", tms);

        printf("%s\n", buff);

        ppkg_receipt_free(receipt);
    } else if (strcmp(key, "receipt-path") == 0) {
        char * userHomeDir = getenv("HOME");

        if (userHomeDir == NULL || strcmp(userHomeDir, "") == 0) {
            return PPKG_ENV_HOME_NOT_SET;
        }

        size_t userHomeDirLength = strlen(userHomeDir);

        size_t  installedDirLength = userHomeDirLength + strlen(packageName) + 20;
        char    installedDir[installedDirLength];
        memset (installedDir, 0, installedDirLength);
        sprintf(installedDir, "%s/.ppkg/installed/%s", userHomeDir, packageName);

        size_t  receiptFilePathLength = installedDirLength + 20;
        char    receiptFilePath[receiptFilePathLength];
        memset (receiptFilePath, 0, receiptFilePathLength);
        sprintf(receiptFilePath, "%s/.ppkg/receipt.yml", installedDir);

        if (exists_and_is_a_regular_file(receiptFilePath)) {
            printf("%s\n", receiptFilePath);
        } else {
            return PPKG_PACKAGE_IS_NOT_INSTALLED;
        }
    } else if (strcmp(key, "receipt-yaml") == 0) {
        char * userHomeDir = getenv("HOME");

        if (userHomeDir == NULL || strcmp(userHomeDir, "") == 0) {
            return PPKG_ENV_HOME_NOT_SET;
        }

        size_t userHomeDirLength = strlen(userHomeDir);

        size_t  installedDirLength = userHomeDirLength + strlen(packageName) + 20;
        char    installedDir[installedDirLength];
        memset (installedDir, 0, installedDirLength);
        sprintf(installedDir, "%s/.ppkg/installed/%s", userHomeDir, packageName);

        size_t  receiptFilePathLength = installedDirLength + 20;
        char    receiptFilePath[receiptFilePathLength];
        memset (receiptFilePath, 0, receiptFilePathLength);
        sprintf(receiptFilePath, "%s/.ppkg/receipt.yml", installedDir);

        if (!exists_and_is_a_regular_file(receiptFilePath)) {
            return PPKG_PACKAGE_IS_NOT_INSTALLED;
        }

        FILE * receiptFile = fopen(receiptFilePath, "r");

        if (receiptFile == NULL) {
            perror(receiptFilePath);
            return PPKG_ERROR;
        }

        char buff[1024];
        int  size = 0;
        while((size = fread(buff, 1, 1024, receiptFile)) != 0) {
            fwrite(buff, 1, size, stdout);
        }

        fclose(receiptFile);
    } else if (strcmp(key, "receipt-json") == 0) {
        PPKGReceipt * receipt = NULL;

        int resultCode = ppkg_receipt_parse(packageName, &receipt);

        if (resultCode != PPKG_OK) {
            return resultCode;
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

        json_object_set_new(root, "cdefine", json_string(receipt->cdefine));
        json_object_set_new(root, "ccflags", json_string(receipt->ccflags));
        json_object_set_new(root, "xxflags", json_string(receipt->xxflags));
        json_object_set_new(root, "ldfalgs", json_string(receipt->ldflags));

        json_object_set_new(root, "bsystem", json_string(receipt->bscript));
        json_object_set_new(root, "bscript", json_string(receipt->bscript));
        json_object_set_new(root, "binbstd", json_boolean(receipt->binbstd));
        json_object_set_new(root, "parallel", json_boolean(receipt->parallel));
        json_object_set_new(root, "symlink", json_boolean(receipt->symlink));

        json_object_set_new(root, "exetype", json_string(receipt->exetype));

        json_object_set_new(root, "prepare", json_string(receipt->prepare));
        json_object_set_new(root, "install", json_string(receipt->install));

        json_object_set_new(root, "signature", json_string(receipt->signature));
        json_object_set_new(root, "timestamp", json_string(receipt->timestamp));

        char * jsonStr = json_dumps(root, 0);

        if (jsonStr == NULL) {
            resultCode = PPKG_ERROR;
        } else {
            printf("%s\n", jsonStr);
            free(jsonStr);
        }

        json_decref(root);

        ppkg_receipt_free(receipt);
    } else {
        return PPKG_ARG_IS_UNKNOWN;
    }

    return resultCode;
}
