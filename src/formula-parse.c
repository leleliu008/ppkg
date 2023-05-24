#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <libgen.h>
#include <time.h>
#include <yaml.h>
#include "ppkg.h"
#include "core/regex/regex.h"

typedef enum {
    FORMULA_KEY_CODE_unknown,

    FORMULA_KEY_CODE_summary,
    FORMULA_KEY_CODE_version,
    FORMULA_KEY_CODE_license,

    FORMULA_KEY_CODE_web_url,

    FORMULA_KEY_CODE_git_url,
    FORMULA_KEY_CODE_git_sha,
    FORMULA_KEY_CODE_git_ref,
    FORMULA_KEY_CODE_shallow,

    FORMULA_KEY_CODE_src_url,
    FORMULA_KEY_CODE_src_sha,

    FORMULA_KEY_CODE_fix_url,
    FORMULA_KEY_CODE_fix_sha,

    FORMULA_KEY_CODE_res_url,
    FORMULA_KEY_CODE_res_sha,

    FORMULA_KEY_CODE_dep_pkg,
    FORMULA_KEY_CODE_dep_upp,
    FORMULA_KEY_CODE_dep_pym,
    FORMULA_KEY_CODE_dep_plm,

    FORMULA_KEY_CODE_bsystem,
    FORMULA_KEY_CODE_bscript,
    FORMULA_KEY_CODE_binbstd,

    FORMULA_KEY_CODE_dopatch,
    FORMULA_KEY_CODE_install,
    FORMULA_KEY_CODE_symlink,

    FORMULA_KEY_CODE_ppflags,
    FORMULA_KEY_CODE_ccflags,
    FORMULA_KEY_CODE_xxflags,
    FORMULA_KEY_CODE_ldflags,

    FORMULA_KEY_CODE_exetype,

    FORMULA_KEY_CODE_parallel,
} PPKGFormulaKeyCode;

void ppkg_formula_dump(PPKGFormula * formula) {
    if (formula == NULL) {
        return;
    }

    printf("summary: %s\n", formula->summary);
    printf("version: %s\n", formula->version);
    printf("license: %s\n", formula->license);

    printf("web-url: %s\n", formula->web_url);

    printf("git_url: %s\n", formula->git_url);
    printf("git_sha: %s\n", formula->git_sha);
    printf("git_ref: %s\n", formula->git_ref);
    printf("shallow: %d\n", formula->shallow);

    printf("src_url: %s\n", formula->src_url);
    printf("src_sha: %s\n", formula->src_sha);

    printf("fix_url: %s\n", formula->fix_url);
    printf("fix_sha: %s\n", formula->fix_sha);

    printf("res_url: %s\n", formula->res_url);
    printf("res_sha: %s\n", formula->res_sha);

    printf("dep_pkg: %s\n", formula->dep_pkg);
    printf("dep_upp: %s\n", formula->dep_upp);
    printf("dep_pym: %s\n", formula->dep_pym);
    printf("dep_plm: %s\n", formula->dep_plm);

    printf("bsystem: %s\n", formula->bsystem);
    printf("bscript: %s\n", formula->bscript);
    printf("binbstd: %d\n", formula->binbstd);
    printf("parallel: %d\n", formula->parallel);

    printf("dopatch: %s\n", formula->dopatch);
    printf("install: %s\n", formula->install);

    printf("symlink: %d\n", formula->symlink);

    printf("exetype: %s\n", formula->exetype);

    printf("path:    %s\n", formula->path);

    printf("useBuildSystemCmake:    %d\n", formula->useBuildSystemCmake);
}

void ppkg_formula_free(PPKGFormula * formula) {
    if (formula == NULL) {
        return;
    }

    if (formula->summary != NULL) {
        free(formula->summary);
        formula->summary = NULL;
    }

    if (formula->version != NULL) {
        free(formula->version);
        formula->version = NULL;
    }

    if (formula->license != NULL) {
        free(formula->license);
        formula->license = NULL;
    }

    if (formula->web_url != NULL) {
        free(formula->web_url);
        formula->web_url = NULL;
    }

    ///////////////////////////////

    if (formula->git_url != NULL) {
        free(formula->git_url);
        formula->git_url = NULL;
    }

    if (formula->git_sha != NULL) {
        free(formula->git_sha);
        formula->git_sha = NULL;
    }

    if (formula->git_ref != NULL) {
        free(formula->git_ref);
        formula->git_ref = NULL;
    }

    ///////////////////////////////

    if (formula->src_url != NULL) {
        free(formula->src_url);
        formula->src_url = NULL;
    }

    if (formula->src_sha != NULL) {
        free(formula->src_sha);
        formula->src_sha = NULL;
    }

    ///////////////////////////////

    if (formula->fix_url != NULL) {
        free(formula->fix_url);
        formula->fix_url = NULL;
    }

    if (formula->fix_sha != NULL) {
        free(formula->fix_sha);
        formula->fix_sha = NULL;
    }

    ///////////////////////////////

    if (formula->res_url != NULL) {
        free(formula->res_url);
        formula->res_url = NULL;
    }

    if (formula->res_sha != NULL) {
        free(formula->res_sha);
        formula->res_sha = NULL;
    }

    ///////////////////////////////

    if (formula->dep_pkg != NULL) {
        free(formula->dep_pkg);
        formula->dep_pkg = NULL;
    }

    if (formula->dep_upp != NULL) {
        free(formula->dep_upp);
        formula->dep_upp = NULL;
    }

    if (formula->dep_pym != NULL) {
        free(formula->dep_pym);
        formula->dep_pym = NULL;
    }

    if (formula->dep_plm != NULL) {
        free(formula->dep_plm);
        formula->dep_plm = NULL;
    }

    ///////////////////////////////

    if (formula->bsystem != NULL) {
        free(formula->bsystem);
        formula->bsystem = NULL;
    }

    if (formula->bscript != NULL) {
        free(formula->bscript);
        formula->bscript = NULL;
    }

    ///////////////////////////////

    if (formula->ppflags != NULL) {
        free(formula->ppflags);
        formula->ppflags = NULL;
    }

    if (formula->ccflags != NULL) {
        free(formula->ccflags);
        formula->ccflags = NULL;
    }

    if (formula->xxflags != NULL) {
        free(formula->xxflags);
        formula->xxflags = NULL;
    }

    if (formula->ldflags != NULL) {
        free(formula->ldflags);
        formula->ldflags = NULL;
    }

    ///////////////////////////////

    if (formula->exetype != NULL) {
        free(formula->exetype);
        formula->exetype = NULL;
    }

    ///////////////////////////////

    if (formula->dopatch != NULL) {
        free(formula->dopatch);
        formula->dopatch = NULL;
    }

    if (formula->install != NULL) {
        free(formula->install);
        formula->install = NULL;
    }

    ///////////////////////////////

    if (formula->path != NULL) {
        free(formula->path);
        formula->path = NULL;
    }

    free(formula);
}

static PPKGFormulaKeyCode ppkg_formula_key_code_from_key_name(char * key) {
           if (strcmp(key, "summary") == 0) {
        return FORMULA_KEY_CODE_summary;
    } else if (strcmp(key, "version") == 0) {
        return FORMULA_KEY_CODE_version;
    } else if (strcmp(key, "license") == 0) {
        return FORMULA_KEY_CODE_license;
    } else if (strcmp(key, "web-url") == 0) {
        return FORMULA_KEY_CODE_web_url;
    } else if (strcmp(key, "git-url") == 0) {
        return FORMULA_KEY_CODE_git_url;
    } else if (strcmp(key, "git-sha") == 0) {
        return FORMULA_KEY_CODE_git_sha;
    } else if (strcmp(key, "git-ref") == 0) {
        return FORMULA_KEY_CODE_git_ref;
    } else if (strcmp(key, "shallow") == 0) {
        return FORMULA_KEY_CODE_shallow;
    } else if (strcmp(key, "src-url") == 0) {
        return FORMULA_KEY_CODE_src_url;
    } else if (strcmp(key, "src-sha") == 0) {
        return FORMULA_KEY_CODE_src_sha;
    } else if (strcmp(key, "fix-url") == 0) {
        return FORMULA_KEY_CODE_fix_url;
    } else if (strcmp(key, "fix-sha") == 0) {
        return FORMULA_KEY_CODE_fix_sha;
    } else if (strcmp(key, "res-url") == 0) {
        return FORMULA_KEY_CODE_res_url;
    } else if (strcmp(key, "res-sha") == 0) {
        return FORMULA_KEY_CODE_res_sha;
    } else if (strcmp(key, "dep-pkg") == 0) {
        return FORMULA_KEY_CODE_dep_pkg;
    } else if (strcmp(key, "dep-upp") == 0) {
        return FORMULA_KEY_CODE_dep_upp;
    } else if (strcmp(key, "dep-pym") == 0) {
        return FORMULA_KEY_CODE_dep_pym;
    } else if (strcmp(key, "dep-plm") == 0) {
        return FORMULA_KEY_CODE_dep_plm;
    } else if (strcmp(key, "ppflags") == 0) {
        return FORMULA_KEY_CODE_ppflags;
    } else if (strcmp(key, "ccflags") == 0) {
        return FORMULA_KEY_CODE_ccflags;
    } else if (strcmp(key, "xxflags") == 0) {
        return FORMULA_KEY_CODE_xxflags;
    } else if (strcmp(key, "ldflags") == 0) {
        return FORMULA_KEY_CODE_ldflags;
    } else if (strcmp(key, "dopatch") == 0) {
        return FORMULA_KEY_CODE_dopatch;
    } else if (strcmp(key, "install") == 0) {
        return FORMULA_KEY_CODE_install;
    } else if (strcmp(key, "symlink") == 0) {
        return FORMULA_KEY_CODE_symlink;
    } else if (strcmp(key, "bsystem") == 0) {
        return FORMULA_KEY_CODE_bsystem;
    } else if (strcmp(key, "bscript") == 0) {
        return FORMULA_KEY_CODE_bscript;
    } else if (strcmp(key, "binbstd") == 0) {
        return FORMULA_KEY_CODE_binbstd;
    } else if (strcmp(key, "exetype") == 0) {
        return FORMULA_KEY_CODE_exetype;
    } else if (strcmp(key, "parallel") == 0) {
        return FORMULA_KEY_CODE_parallel;
    } else {
        return FORMULA_KEY_CODE_unknown;
    }
}

static void ppkg_formula_set_value(PPKGFormulaKeyCode keyCode, char * value, PPKGFormula * formula) {
    if (keyCode == FORMULA_KEY_CODE_unknown) {
        return;
    }

    char c;

    for (;;) {
        c = value[0];

        if (c == '\0') {
            return;
        }

        // non-printable ASCII characters and space
        if (c <= 32) {
            value = &value[1];
        } else {
            break;
        }
    }

    switch (keyCode) {
        case FORMULA_KEY_CODE_summary: if (formula->summary != NULL) free(formula->summary); formula->summary = strdup(value); break;
        case FORMULA_KEY_CODE_version: if (formula->version != NULL) free(formula->version); formula->version = strdup(value); break;
        case FORMULA_KEY_CODE_license: if (formula->license != NULL) free(formula->license); formula->license = strdup(value); break;

        case FORMULA_KEY_CODE_web_url: if (formula->web_url != NULL) free(formula->web_url); formula->web_url = strdup(value); break;

        case FORMULA_KEY_CODE_git_url: if (formula->git_url != NULL) free(formula->git_url); formula->git_url = strdup(value); break;
        case FORMULA_KEY_CODE_git_sha: if (formula->git_sha != NULL) free(formula->git_sha); formula->git_sha = strdup(value); break;
        case FORMULA_KEY_CODE_git_ref: if (formula->git_ref != NULL) free(formula->git_ref); formula->git_ref = strdup(value); break;

        case FORMULA_KEY_CODE_src_url: if (formula->src_url != NULL) free(formula->src_url); formula->src_url = strdup(value); break;
        case FORMULA_KEY_CODE_src_sha: if (formula->src_sha != NULL) free(formula->src_sha); formula->src_sha = strdup(value); break;

        case FORMULA_KEY_CODE_fix_url: if (formula->fix_url != NULL) free(formula->fix_url); formula->fix_url = strdup(value); break;
        case FORMULA_KEY_CODE_fix_sha: if (formula->fix_sha != NULL) free(formula->fix_sha); formula->fix_sha = strdup(value); break;

        case FORMULA_KEY_CODE_res_url: if (formula->res_url != NULL) free(formula->res_url); formula->res_url = strdup(value); break;
        case FORMULA_KEY_CODE_res_sha: if (formula->res_sha != NULL) free(formula->res_sha); formula->res_sha = strdup(value); break;

        case FORMULA_KEY_CODE_dep_pkg: if (formula->dep_pkg != NULL) free(formula->dep_pkg); formula->dep_pkg = strdup(value); break;
        case FORMULA_KEY_CODE_dep_upp: if (formula->dep_upp != NULL) free(formula->dep_upp); formula->dep_upp = strdup(value); break;
        case FORMULA_KEY_CODE_dep_pym: if (formula->dep_pym != NULL) free(formula->dep_pym); formula->dep_pym = strdup(value); break;
        case FORMULA_KEY_CODE_dep_plm: if (formula->dep_plm != NULL) free(formula->dep_plm); formula->dep_plm = strdup(value); break;

        case FORMULA_KEY_CODE_ppflags: if (formula->ppflags != NULL) free(formula->ppflags); formula->ppflags = strdup(value); break;
        case FORMULA_KEY_CODE_ccflags: if (formula->ccflags != NULL) free(formula->ccflags); formula->ccflags = strdup(value); break;
        case FORMULA_KEY_CODE_xxflags: if (formula->xxflags != NULL) free(formula->xxflags); formula->xxflags = strdup(value); break;
        case FORMULA_KEY_CODE_ldflags: if (formula->ldflags != NULL) free(formula->ldflags); formula->ldflags = strdup(value); break;

        case FORMULA_KEY_CODE_dopatch: if (formula->dopatch != NULL) free(formula->dopatch); formula->dopatch = strdup(value); break;
        case FORMULA_KEY_CODE_install: if (formula->install != NULL) free(formula->install); formula->install = strdup(value); break;

        case FORMULA_KEY_CODE_exetype: if (formula->exetype != NULL) free(formula->exetype); formula->exetype = strdup(value); break;

        case FORMULA_KEY_CODE_bsystem: if (formula->bsystem != NULL) free(formula->bsystem); formula->bsystem = strdup(value); break;
        case FORMULA_KEY_CODE_bscript: if (formula->bscript != NULL) free(formula->bscript); formula->bscript = strdup(value); break;

        case FORMULA_KEY_CODE_shallow:
            if (strcmp(value, "yes") == 0) {
                formula->shallow = true;
            } else if (strcmp(value, "no") == 0) {
                formula->shallow = false;
            } else {
                //TODO
            }
            break;
        case FORMULA_KEY_CODE_binbstd:
            if (strcmp(value, "yes") == 0) {
                formula->binbstd = true;
            } else if (strcmp(value, "no") == 0) {
                formula->binbstd = false;
            } else {
                //TODO
            }
            break;
        case FORMULA_KEY_CODE_symlink:
            if (strcmp(value, "yes") == 0) {
                formula->symlink = true;
            } else if (strcmp(value, "no") == 0) {
                formula->symlink = false;
            } else {
                //TODO
            }
            break;
        case FORMULA_KEY_CODE_parallel:
            if (strcmp(value, "yes") == 0) {
                formula->parallel = true;
            } else if (strcmp(value, "no") == 0) {
                formula->parallel = false;
            } else {
                //TODO
            }
            break;
        default:
            break;
    }
}

static inline bool ppkg_formula_check_bsystem_match(PPKGFormula * formula, const char * buf, size_t bufLength) {
    if (bufLength == 0) {
        return false;
    } else if (regex_matched(buf, "^[[:space:]]*configure[[:space:]]*")) {
        formula->bsystem = strdup("configure");
        formula->bsystem_is_calculated = true;
        return true;
    } else if (regex_matched(buf, "^[[:space:]]*cmakew[[:space:]]*")) {
        formula->bsystem = strdup("cmake");
        formula->bsystem_is_calculated = true;
        return true;
    } else if (regex_matched(buf, "^[[:space:]]*xmakew[[:space:]]*")) {
        formula->bsystem = strdup("xmake");
        formula->bsystem_is_calculated = true;
        return true;
    } else if (regex_matched(buf, "^[[:space:]]*gmakew[[:space:]]*")) {
        formula->bsystem = strdup("gmake");
        formula->bsystem_is_calculated = true;
        return true;
    } else if (regex_matched(buf, "^[[:space:]]*mesonw[[:space:]]*")) {
        formula->bsystem = strdup("meson");
        formula->bsystem_is_calculated = true;
        return true;
    } else if (regex_matched(buf, "^[[:space:]]*cargow[[:space:]]*")) {
        formula->bsystem = strdup("cargo");
        formula->bsystem_is_calculated = true;
        return true;
    } else if (regex_matched(buf, "^[[:space:]]*gow?[[:space:]]*")) {
        formula->bsystem = strdup("go");
        formula->bsystem_is_calculated = true;
        return true;
    } else {
        return false;
    }
}

static int ppkg_formula_check_bsystem(PPKGFormula * formula) {
    if (formula->bsystem != NULL) {
        return PPKG_OK;
    }

    if (formula->install == NULL) {
        fprintf(stderr, "scheme error in formula file: %s : neither bsystem nor install mapping is found.\n", formula->path);
        return PPKG_ERROR_FORMULA_SCHEME;
    }

    const char * installString = formula->install;

    char   buf[11] = {0};
    size_t bufLength = 0;

    size_t i = 0;

    char c;

    for (;;) {
        c = installString[i];

        if (c == '\0') {
            ppkg_formula_check_bsystem_match(formula, buf, bufLength);
            return PPKG_OK;
        }

        if (c == '\n') {
            if (ppkg_formula_check_bsystem_match(formula, buf, bufLength)) {
                return PPKG_OK;
            } else {
                bufLength = 0;
                i++;
                continue;
            }
        }

        if (bufLength == 10) {
            if (ppkg_formula_check_bsystem_match(formula, buf, bufLength)) {
                return PPKG_OK;
            } else {
                i++;
                continue;
            }
        }

        if (bufLength == 0) {
            memset(buf, 0, 11);
        }

        buf[bufLength] = c;
        bufLength++;

        i++;
    }
}

static int ppkg_formula_check(PPKGFormula * formula, const char * formulaFilePath) {
    if (formula->summary == NULL) {
        fprintf(stderr, "scheme error in formula file: %s : summary mapping not found.\n", formulaFilePath);
        return PPKG_ERROR_FORMULA_SCHEME;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->web_url == NULL) {
        if (formula->git_url == NULL) {
            fprintf(stderr, "scheme error in formula file: %s : web-url mapping not found.\n", formulaFilePath);
            return PPKG_ERROR_FORMULA_SCHEME;
        } else {
            formula->web_url = strdup(formula->git_url);
            formula->web_url_is_calculated = true;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->src_url == NULL) {
        if (formula->git_url == NULL) {
            fprintf(stderr, "scheme error in formula file: %s : neither src-url nor git-url mapping is found.\n", formulaFilePath);
            return PPKG_ERROR_FORMULA_SCHEME;
        } else {
            if ((formula->git_sha != NULL) && (strlen(formula->git_sha) != 40)) {
                fprintf(stderr, "scheme error in formula file: %s : git-sha mapping's value's length must be 40.\n", formulaFilePath);
                return PPKG_ERROR_FORMULA_SCHEME;
            }
        }
    } else {
        size_t i = 0;
        char   c;

        for (;;) {
            c = formula->src_url[i];

            if (c == '\0') {
                break;
            }

            if (i == 5) {
                if (strncmp(formula->src_url, "dir://", 6) == 0) {
                    struct stat st;

                    if (stat(&formula->src_url[6], &st) != 0 || !S_ISDIR(st.st_mode)) {
                        fprintf(stderr, "src-url mapping request local dir %s not exist. in formula file: %s.\n", &formula->src_url[6], formulaFilePath);
                        return PPKG_ERROR;
                    }

                    formula->src_is_dir = true;
                }

                break;
            }

            i++;
        }

        if (!(formula->src_is_dir)) {
            if (formula->src_sha == NULL) {
                fprintf(stderr, "scheme error in formula file: %s : src-sha mapping not found.\n", formulaFilePath);
                return PPKG_ERROR_FORMULA_SCHEME;
            }

            if (strlen(formula->src_sha) != 64) {
                fprintf(stderr, "scheme error in formula file: %s : src-sha mapping's value's length must be 64.\n", formulaFilePath);
                return PPKG_ERROR_FORMULA_SCHEME;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->fix_url != NULL) {
        if (formula->fix_sha == NULL) {
            fprintf(stderr, "scheme error in formula file: %s : fix-sha mapping not found.\n", formulaFilePath);
            return PPKG_ERROR_FORMULA_SCHEME;
        }

        if (strlen(formula->fix_sha) != 64) {
            fprintf(stderr, "scheme error in formula file: %s : fix-sha mapping's value's length must be 64.\n", formulaFilePath);
            return PPKG_ERROR_FORMULA_SCHEME;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->res_url != NULL) {
        if (formula->res_sha == NULL) {
            fprintf(stderr, "scheme error in formula file: %s : res-sha mapping not found.\n", formulaFilePath);
            return PPKG_ERROR_FORMULA_SCHEME;
        }

        if (strlen(formula->res_sha) != 64) {
            fprintf(stderr, "scheme error in formula file: %s : res-sha mapping's value's length must be 64.\n", formulaFilePath);
            return PPKG_ERROR_FORMULA_SCHEME;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->version == NULL) {
        if ((formula->src_url != NULL) && (!formula->src_is_dir)) {
            size_t urlLength = strlen(formula->src_url);
            size_t urlCopyLength = urlLength + 1U;
            char   urlCopy[urlCopyLength];
            strncpy(urlCopy, formula->src_url, urlCopyLength);

            char * srcFileName = basename(urlCopy);

            size_t srcFileNameLength = 0;

            char c;

            for (;;) {
                c = srcFileName[srcFileNameLength];

                if (c == '\0') {
                    break;
                }

                if (c == '_' || c == '@') {
                    srcFileName[srcFileNameLength] = '-';
                }

                srcFileNameLength++;
            }

            if (srcFileNameLength > 8) {
                       if (strcmp(&srcFileName[srcFileNameLength - 8], ".tar.bz2") == 0) {
                    srcFileName[srcFileNameLength - 8] = '\0';
                    srcFileNameLength -= 8;
                } else if (strcmp(&srcFileName[srcFileNameLength - 7], ".tar.gz") == 0) {
                    srcFileName[srcFileNameLength - 7] = '\0';
                    srcFileNameLength -= 7;
                } else if (strcmp(&srcFileName[srcFileNameLength - 7], ".tar.xz") == 0) {
                    srcFileName[srcFileNameLength - 7] = '\0';
                    srcFileNameLength -= 7;
                } else if (strcmp(&srcFileName[srcFileNameLength - 7], ".tar.lz") == 0) {
                    srcFileName[srcFileNameLength - 7] = '\0';
                    srcFileNameLength -= 7;
                } else if (strcmp(&srcFileName[srcFileNameLength - 5], ".tbz2") == 0) {
                    srcFileName[srcFileNameLength - 5] = '\0';
                    srcFileNameLength -= 5;
                } else if (strcmp(&srcFileName[srcFileNameLength - 4], ".tgz") == 0) {
                    srcFileName[srcFileNameLength - 4] = '\0';
                    srcFileNameLength -= 4;
                } else if (strcmp(&srcFileName[srcFileNameLength - 4], ".txz") == 0) {
                    srcFileName[srcFileNameLength - 4] = '\0';
                    srcFileNameLength -= 4;
                } else if (strcmp(&srcFileName[srcFileNameLength - 4], ".tlz") == 0) {
                    srcFileName[srcFileNameLength - 4] = '\0';
                    srcFileNameLength -= 4;
                } else if (strcmp(&srcFileName[srcFileNameLength - 4], ".zip") == 0) {
                    srcFileName[srcFileNameLength - 4] = '\0';
                    srcFileNameLength -= 4;
                } else {
                    fprintf(stderr, "scheme error in formula file: %s : unsupported filename extension.\n", formulaFilePath);
                    return PPKG_ERROR_FORMULA_SCHEME;
                }
            } else if (srcFileNameLength > 7) {
                       if (strcmp(&srcFileName[srcFileNameLength - 7], ".tar.gz") == 0) {
                    srcFileName[srcFileNameLength - 7] = '\0';
                    srcFileNameLength -= 7;
                } else if (strcmp(&srcFileName[srcFileNameLength - 7], ".tar.xz") == 0) {
                    srcFileName[srcFileNameLength - 7] = '\0';
                    srcFileNameLength -= 7;
                } else if (strcmp(&srcFileName[srcFileNameLength - 7], ".tar.lz") == 0) {
                    srcFileName[srcFileNameLength - 7] = '\0';
                    srcFileNameLength -= 7;
                } else {
                    fprintf(stderr, "scheme error in formula file: %s : unsupported filename extension.\n", formulaFilePath);
                    return PPKG_ERROR_FORMULA_SCHEME;
                }
            } else if (srcFileNameLength > 5) {
                       if (strcmp(&srcFileName[srcFileNameLength - 5], ".tbz2") == 0) {
                    srcFileName[srcFileNameLength - 5] = '\0';
                    srcFileNameLength -= 5;
                } else {
                    fprintf(stderr, "scheme error in formula file: %s : unsupported filename extension.\n", formulaFilePath);
                    return PPKG_ERROR_FORMULA_SCHEME;
                }
            } else if (srcFileNameLength > 4) {
                       if (strcmp(&srcFileName[srcFileNameLength - 4], ".tgz") == 0) {
                    srcFileName[srcFileNameLength - 4] = '\0';
                    srcFileNameLength -= 4;
                } else if (strcmp(&srcFileName[srcFileNameLength - 4], ".txz") == 0) {
                    srcFileName[srcFileNameLength - 4] = '\0';
                    srcFileNameLength -= 4;
                } else if (strcmp(&srcFileName[srcFileNameLength - 4], ".tlz") == 0) {
                    srcFileName[srcFileNameLength - 4] = '\0';
                    srcFileNameLength -= 4;
                } else if (strcmp(&srcFileName[srcFileNameLength - 4], ".zip") == 0) {
                    srcFileName[srcFileNameLength - 4] = '\0';
                    srcFileNameLength -= 4;
                } else {
                    fprintf(stderr, "scheme error in formula file: %s : unsupported filename extension.\n", formulaFilePath);
                    return PPKG_ERROR_FORMULA_SCHEME;
                }
            }

            //printf("----------------srcFileName = %s\n", srcFileName);

            if (srcFileNameLength > 5) {
                       if (strcmp(&srcFileName[srcFileNameLength - 5], ".orig") == 0) {
                    srcFileName[srcFileNameLength - 5] = '\0';
                    srcFileNameLength -= 5;
                } else if (strcmp(&srcFileName[srcFileNameLength - 4], ".src") == 0) {
                    srcFileName[srcFileNameLength - 4] = '\0';
                    srcFileNameLength -= 4;
                }
            } else if (srcFileNameLength > 4) {
                       if (strcmp(&srcFileName[srcFileNameLength - 4], ".src") == 0) {
                    srcFileName[srcFileNameLength - 4] = '\0';
                    srcFileNameLength -= 4;
                }
            }

            //printf("----------------srcFileName = %s\n", srcFileName);

            char * splitedStr = strtok(srcFileName, "-");

            while (splitedStr != NULL) {
                if (regex_matched(splitedStr, "^[0-9]+(\\.[0-9]+)+[a-z]?$")) {
                    formula->version = strdup(splitedStr);
                    formula->version_is_calculated = true;
                    break;
                } else if (regex_matched(splitedStr, "^[vV][0-9]+(\\.[0-9]+)+[a-z]?$")) {
                    formula->version = strdup(&splitedStr[1]);
                    formula->version_is_calculated = true;
                    break;
                } else if (regex_matched(splitedStr, "^[0-9]{3,8}$")) {
                    formula->version = strdup(splitedStr);
                    formula->version_is_calculated = true;
                    break;
                } else if (regex_matched(splitedStr, "^[vrR][0-9]{2,8}$")) {
                    formula->version = strdup(&splitedStr[1]);
                    formula->version_is_calculated = true;
                    break;
                }

                splitedStr = strtok(NULL, "-");
            }

            if (formula->version == NULL) {
                fprintf(stderr, "scheme error in formula file: %s : version mapping not found.\n", formulaFilePath);
                return PPKG_ERROR_FORMULA_SCHEME;
            }
        } else {
            time_t tt = time(NULL);
            struct tm *tms = localtime(&tt);

            formula->version = (char*)calloc(11, sizeof(char));

            if (formula->version == NULL) {
                return PPKG_ERROR_MEMORY_ALLOCATE;
            }

            strftime(formula->version, 11, "%Y.%m.%d", tms);

            formula->version_is_calculated = true;
        }
    }

    int ret = ppkg_formula_check_bsystem(formula);

    if (ret != PPKG_OK) {
        return ret;
    }

    if (formula->bsystem == NULL) {
        fprintf(stderr, "scheme error in formula file: %s : bsystem mapping not found.\n", formula->path);
        return PPKG_ERROR_FORMULA_SCHEME;
    }

    size_t  bsystemStrLenth = strlen(formula->bsystem) + 1U;
    char    bsystemStr[bsystemStrLenth];
    strncpy(bsystemStr, formula->bsystem, bsystemStrLenth);

    char * bsystem = strtok(bsystemStr, " ");

    while (bsystem != NULL) {
               if (strcmp(bsystem, "autogen") == 0) {
            formula->useBuildSystemAutogen = true;
            formula->useBuildSystemGmake = true;
        } else if (strcmp(bsystem, "autotools") == 0) {
            formula->useBuildSystemAutotools = true;
            formula->useBuildSystemGmake = true;
        } else if (strcmp(bsystem, "configure") == 0) {
            formula->useBuildSystemConfigure = true;
            formula->useBuildSystemGmake = true;
        } else if (strcmp(bsystem, "cmake") == 0) {
            formula->useBuildSystemCmake = true;
            formula->useBuildSystemNinja = true;
        } else if (strcmp(bsystem, "cmake-ninja") == 0) {
            formula->useBuildSystemCmake = true;
            formula->useBuildSystemNinja = true;
        } else if (strcmp(bsystem, "cmake-gmake") == 0) {
            formula->useBuildSystemCmake = true;
            formula->useBuildSystemGmake = true;
        } else if (strcmp(bsystem, "xmake") == 0) {
            formula->useBuildSystemXmake = true;
        } else if (strcmp(bsystem, "gmake") == 0) {
            formula->useBuildSystemGmake = true;
        } else if (strcmp(bsystem, "ninja") == 0) {
            formula->useBuildSystemNinja = true;
        } else if (strcmp(bsystem, "meson") == 0) {
            formula->useBuildSystemMeson = true;
            formula->useBuildSystemNinja = true;
        } else if (strcmp(bsystem, "cargo") == 0) {
            formula->useBuildSystemCargo = true;
        } else if (strcmp(bsystem, "go") == 0) {
            formula->useBuildSystemGolang = true;
        }

        bsystem = strtok(NULL, " ");
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->install == NULL) {
        const char * dobuildActions;

        if (formula->useBuildSystemAutogen) {
            dobuildActions = "configure";
        } else if (formula->useBuildSystemAutotools) {
            dobuildActions = "configure";
        } else if (formula->useBuildSystemConfigure) {
            dobuildActions = "configure";
        } else if (formula->useBuildSystemCmake) {
            dobuildActions = "cmakew";
        } else if (formula->useBuildSystemXmake) {
            dobuildActions = "xmakew";
        } else if (formula->useBuildSystemMeson) {
            dobuildActions = "mesonw";
        } else if (formula->useBuildSystemNinja) {
            dobuildActions = "ninjaw clean && ninjaw && ninjaw install";
        } else if (formula->useBuildSystemGmake) {
            dobuildActions = "gmakew clean && gmakew && gmakew install";
        } else if (formula->useBuildSystemCargo) {
            dobuildActions = "cargow install";
        } else if (formula->useBuildSystemGolang) {
            dobuildActions = "gow";
        } else {
            fprintf(stderr, "scheme error in formula file: %s : dobuild mapping not found.\n", formula->path);
            return PPKG_ERROR_FORMULA_SCHEME;
        }

        formula->install = strdup(dobuildActions);

        if (formula->install == NULL) {
            return PPKG_ERROR_MEMORY_ALLOCATE;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemXmake || formula->useBuildSystemCargo || formula->useBuildSystemGolang) {
        formula->binbstd = true;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    return PPKG_OK;
}

int ppkg_formula_parse(const char * formulaFilePath, PPKGFormula * * out) {
    FILE * file = fopen(formulaFilePath, "r");

    if (file == NULL) {
        perror(formulaFilePath);
        return PPKG_ERROR;
    }

    yaml_parser_t parser;
    yaml_token_t  token;

    // https://libyaml.docsforge.com/master/api/yaml_parser_initialize/
    if (yaml_parser_initialize(&parser) == 0) {
        perror("Failed to initialize yaml parser");
        return PPKG_ERROR;
    }

    yaml_parser_set_input_file(&parser, file);

    PPKGFormulaKeyCode formulaKeyCode = FORMULA_KEY_CODE_unknown;

    PPKGFormula * formula = NULL;

    int lastTokenType = 0;

    int ret = PPKG_OK;

    do {
        // https://libyaml.docsforge.com/master/api/yaml_parser_scan/
        if (yaml_parser_scan(&parser, &token) == 0) {
            fprintf(stderr, "syntax error in formula file: %s\n", formulaFilePath);
            ret = PPKG_ERROR_FORMULA_SYNTAX;
            goto finalize;
        }

        switch(token.type) {
            case YAML_KEY_TOKEN:
                lastTokenType = 1;
                break;
            case YAML_VALUE_TOKEN:
                lastTokenType = 2;
                break;
            case YAML_SCALAR_TOKEN:
                if (lastTokenType == 1) {
                    formulaKeyCode = ppkg_formula_key_code_from_key_name((char*)token.data.scalar.value);
                } else if (lastTokenType == 2) {
                    if (formula == NULL) {
                        formula = (PPKGFormula*)calloc(1, sizeof(PPKGFormula));

                        if (formula == NULL) {
                            ret = PPKG_ERROR_MEMORY_ALLOCATE;
                            goto finalize;
                        }

                        formula->path = strdup(formulaFilePath);

                        if (formula->path == NULL) {
                            ret = PPKG_ERROR_MEMORY_ALLOCATE;
                            goto finalize;
                        }

                        formula->shallow = true;
                        formula->symlink = true;
                        formula->binbstd = false;
                        formula->parallel = true;
                    }

                    ppkg_formula_set_value(formulaKeyCode, (char*)token.data.scalar.value, formula);
                }
                break;
            default: 
                lastTokenType = 0;
                break;
        }

        if (token.type != YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&token);
        }
    } while(token.type != YAML_STREAM_END_TOKEN);

finalize:
    yaml_token_delete(&token);

    yaml_parser_delete(&parser);

    fclose(file);

    if (ret == PPKG_OK) {
        ret = ppkg_formula_check(formula, formulaFilePath);

        if (ret == PPKG_OK) {
            (*out) = formula;
            return PPKG_OK;
        }
    }

    ppkg_formula_free(formula);
    return ret;
}
