#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include <jansson.h>

#include "core/log.h"

#include "ppkg.h"

int ppkg_available_info(const char * packageName, const char * targetPlatformName, const char * key) {
    int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    if ((key == NULL) || (key[0] == '\0') || (strcmp(key, "--yaml") == 0)) {
        char * formulaFilePath = NULL;

        ret = ppkg_formula_locate(packageName, targetPlatformName, &formulaFilePath);

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
            printf("pkgname: %s%s%s\n", COLOR_GREEN, packageName, COLOR_OFF);
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
    } else if (strcmp(key, "--json") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

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
        json_object_set_new(root, "git-nth", json_integer(formula->git_nth));

        json_object_set_new(root, "src-url", json_string(formula->src_url));
        json_object_set_new(root, "src-uri", json_string(formula->src_uri));
        json_object_set_new(root, "src-sha", json_string(formula->src_sha));

        json_object_set_new(root, "fix-url", json_string(formula->fix_url));
        json_object_set_new(root, "fix-uri", json_string(formula->fix_uri));
        json_object_set_new(root, "fix-sha", json_string(formula->fix_sha));

        json_object_set_new(root, "res-url", json_string(formula->res_url));
        json_object_set_new(root, "res-uri", json_string(formula->res_uri));
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

        json_object_set_new(root, "do12345", json_string(formula->do12345));
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
    } else if (strcmp(key, "formula") == 0) {
        char * formulaFilePath = NULL;

        ret = ppkg_formula_locate(packageName, targetPlatformName, &formulaFilePath);

        if (ret != PPKG_OK) {
            return ret;
        }

        printf("%s\n", formulaFilePath);
        free(formulaFilePath);
    } else if (strcmp(key, "summary") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->summary != NULL) {
            printf("%s\n", formula->summary);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "version") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->version != NULL) {
            printf("%s\n", formula->version);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "license") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->license != NULL) {
            printf("%s\n", formula->license);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "web-url") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->web_url != NULL) {
            printf("%s\n", formula->web_url);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "git-url") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->git_url != NULL) {
            printf("%s\n", formula->git_url);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "git-sha") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->git_sha != NULL) {
            printf("%s\n", formula->git_sha);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "git-ref") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->git_ref != NULL) {
            printf("%s\n", formula->git_ref);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "git-nth") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        printf("%zu\n", formula->git_nth);

        ppkg_formula_free(formula);
    } else if (strcmp(key, "src-url") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->src_url != NULL) {
            printf("%s\n", formula->src_url);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "src-sha") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->src_sha != NULL) {
            printf("%s\n", formula->src_sha);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "src-ft") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        char fileNameExtension[21] = {0};

        ret = ppkg_examine_filetype_from_url(formula->src_url, fileNameExtension, 20);

        if (ret != PPKG_OK) {
            ppkg_formula_free(formula);
            return ret;
        }

        printf("%s\n", fileNameExtension);

        ppkg_formula_free(formula);
    } else if (strcmp(key, "src-fp") == 0) {
        char   ppkgHomeDIR[PATH_MAX];
        size_t ppkgHomeDIRLength;

        int ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

        if (ret != PPKG_OK) {
            return ret;
        }

        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        char fileNameExtension[21] = {0};

        ret = ppkg_examine_filetype_from_url(formula->src_url, fileNameExtension, 20);

        if (ret != PPKG_OK) {
            ppkg_formula_free(formula);
            return ret;
        }

        printf("%s/downloads/%s%s\n", ppkgHomeDIR, formula->src_sha, fileNameExtension);

        ppkg_formula_free(formula);
    } else if (strcmp(key, "fix-url") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->fix_url != NULL) {
            printf("%s\n", formula->fix_url);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "fix-sha") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->fix_sha != NULL) {
            printf("%s\n", formula->fix_sha);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "fix-ft") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        char fileNameExtension[21] = {0};

        ret = ppkg_examine_filetype_from_url(formula->fix_url, fileNameExtension, 20);

        if (ret != PPKG_OK) {
            ppkg_formula_free(formula);
            return ret;
        }

        printf("%s\n", fileNameExtension);

        ppkg_formula_free(formula);
    } else if (strcmp(key, "fix-fp") == 0) {
        char   ppkgHomeDIR[PATH_MAX];
        size_t ppkgHomeDIRLength;

        int ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

        if (ret != PPKG_OK) {
            return ret;
        }

        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        char fileNameExtension[21] = {0};

        ret = ppkg_examine_filetype_from_url(formula->fix_url, fileNameExtension, 20);

        if (ret != PPKG_OK) {
            ppkg_formula_free(formula);
            return ret;
        }

        printf("%s/downloads/%s%s\n", ppkgHomeDIR, formula->fix_sha, fileNameExtension);

        ppkg_formula_free(formula);
    } else if (strcmp(key, "res-url") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->res_url != NULL) {
            printf("%s\n", formula->res_url);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "res-sha") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->res_sha != NULL) {
            printf("%s\n", formula->res_sha);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "res-ft") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        char fileNameExtension[21] = {0};

        ret = ppkg_examine_filetype_from_url(formula->res_url, fileNameExtension, 20);

        if (ret != PPKG_OK) {
            ppkg_formula_free(formula);
            return ret;
        }

        printf("%s\n", fileNameExtension);

        ppkg_formula_free(formula);
    } else if (strcmp(key, "res-fp") == 0) {
        char   ppkgHomeDIR[PATH_MAX];
        size_t ppkgHomeDIRLength;

        int ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

        if (ret != PPKG_OK) {
            return ret;
        }

        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        char fileNameExtension[21] = {0};

        ret = ppkg_examine_filetype_from_url(formula->res_url, fileNameExtension, 20);

        if (ret != PPKG_OK) {
            ppkg_formula_free(formula);
            return ret;
        }

        printf("%s/downloads/%s%s\n", ppkgHomeDIR, formula->res_sha, fileNameExtension);

        ppkg_formula_free(formula);
    } else if (strcmp(key, "dep-pkg") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->dep_pkg != NULL) {
            printf("%s\n", formula->dep_pkg);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "dep-upp") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->dep_upp != NULL) {
            printf("%s\n", formula->dep_upp);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "dep-pym") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->dep_pym != NULL) {
            printf("%s\n", formula->dep_pym);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "dep-plm") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->dep_plm != NULL) {
            printf("%s\n", formula->dep_plm);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "bsystem") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->bsystem != NULL) {
            printf("%s\n", formula->bsystem);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "bscript") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->bscript != NULL) {
            printf("%s\n", formula->bscript);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "binbstd") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        printf("%d\n", formula->binbstd);

        ppkg_formula_free(formula);
    } else if (strcmp(key, "symlink") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        printf("%d\n", formula->symlink);

        ppkg_formula_free(formula);
    } else if (strcmp(key, "parallel") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        printf("%d\n", formula->parallel);

        ppkg_formula_free(formula);
    } else if (strcmp(key, "ppflags") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->ppflags != NULL) {
            printf("%s\n", formula->ppflags);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "ccflags") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->ccflags != NULL) {
            printf("%s\n", formula->ccflags);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "xxflags") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->xxflags != NULL) {
            printf("%s\n", formula->xxflags);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "ldflags") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->ldflags != NULL) {
            printf("%s\n", formula->ldflags);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "do12345") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->do12345 != NULL) {
            printf("%s\n", formula->do12345);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "dopatch") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->dopatch != NULL) {
            printf("%s\n", formula->dopatch);
        }

        ppkg_formula_free(formula);
    } else if (strcmp(key, "install") == 0) {
        PPKGFormula * formula = NULL;

        ret = ppkg_formula_lookup(packageName, targetPlatformName, &formula);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (formula->install != NULL) {
            printf("%s\n", formula->install);
        }

        ppkg_formula_free(formula);
    } else {
        return PPKG_ERROR_ARG_IS_UNKNOWN;
    }

    return ret;
}
