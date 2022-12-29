#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <yaml.h>
#include "ppkg.h"
#include "core/regex/regex.h"

typedef enum {
    RECEIPT_KEY_CODE_unknown,

    RECEIPT_KEY_CODE_summary,
    RECEIPT_KEY_CODE_version,
    RECEIPT_KEY_CODE_license,

    RECEIPT_KEY_CODE_web_url,

    RECEIPT_KEY_CODE_git_url,
    RECEIPT_KEY_CODE_git_sha,
    RECEIPT_KEY_CODE_git_ref,
    RECEIPT_KEY_CODE_shallow,

    RECEIPT_KEY_CODE_src_url,
    RECEIPT_KEY_CODE_src_sha,

    RECEIPT_KEY_CODE_fix_url,
    RECEIPT_KEY_CODE_fix_sha,

    RECEIPT_KEY_CODE_res_url,
    RECEIPT_KEY_CODE_res_sha,

    RECEIPT_KEY_CODE_dep_pkg,
    RECEIPT_KEY_CODE_dep_upp,
    RECEIPT_KEY_CODE_dep_pym,
    RECEIPT_KEY_CODE_dep_plm,

    RECEIPT_KEY_CODE_bsystem,
    RECEIPT_KEY_CODE_bscript,
    RECEIPT_KEY_CODE_binbstd,

    RECEIPT_KEY_CODE_prepare,
    RECEIPT_KEY_CODE_install,
    RECEIPT_KEY_CODE_symlink,

    RECEIPT_KEY_CODE_cdefine,
    RECEIPT_KEY_CODE_ccflags,
    RECEIPT_KEY_CODE_xxflags,
    RECEIPT_KEY_CODE_ldflags,

    RECEIPT_KEY_CODE_exetype,

    RECEIPT_KEY_CODE_parallel,

    RECEIPT_KEY_CODE_signature,
    RECEIPT_KEY_CODE_timestamp,
} PPKGReceiptKeyCode;

void ppkg_receipt_dump(PPKGReceipt * receipt) {
    if (receipt == NULL) {
        return;
    }

    printf("summary: %s\n", receipt->summary);
    printf("version: %s\n", receipt->version);
    printf("license: %s\n", receipt->license);

    printf("web-url: %s\n", receipt->web_url);

    printf("git_url: %s\n", receipt->git_url);
    printf("git_sha: %s\n", receipt->git_sha);
    printf("git_ref: %s\n", receipt->git_ref);
    printf("shallow: %d\n", receipt->shallow);

    printf("src_url: %s\n", receipt->src_url);
    printf("src_sha: %s\n", receipt->src_sha);

    printf("res_url: %s\n", receipt->res_url);
    printf("res_sha: %s\n", receipt->res_sha);

    printf("dep_pkg: %s\n", receipt->dep_pkg);
    printf("dep_upp: %s\n", receipt->dep_upp);
    printf("dep_pym: %s\n", receipt->dep_pym);
    printf("dep_plm: %s\n", receipt->dep_plm);

    printf("bsystem: %s\n", receipt->bsystem);
    printf("bscript: %s\n", receipt->bscript);
    printf("binbstd: %d\n", receipt->binbstd);
    printf("parallel: %d\n", receipt->parallel);

    printf("prepare: %s\n", receipt->prepare);
    printf("install: %s\n", receipt->install);

    printf("symlink: %d\n", receipt->symlink);

    printf("exetype: %s\n", receipt->exetype);

    printf("path:    %s\n", receipt->path);

    printf("signature: %s\n", receipt->signature);
    printf("timestamp: %s\n", receipt->timestamp);
}

void ppkg_receipt_free(PPKGReceipt * receipt) {
    if (receipt == NULL) {
        return;
    }

    if (receipt->summary != NULL) {
        free(receipt->summary);
        receipt->summary = NULL;
    }

    if (receipt->version != NULL) {
        free(receipt->version);
        receipt->version = NULL;
    }

    if (receipt->license != NULL) {
        free(receipt->license);
        receipt->license = NULL;
    }

    if (receipt->web_url != NULL) {
        free(receipt->web_url);
        receipt->web_url = NULL;
    }

    ///////////////////////////////

    if (receipt->git_url != NULL) {
        free(receipt->git_url);
        receipt->git_url = NULL;
    }

    if (receipt->git_sha != NULL) {
        free(receipt->git_sha);
        receipt->git_sha = NULL;
    }

    if (receipt->git_ref != NULL) {
        free(receipt->git_ref);
        receipt->git_ref = NULL;
    }

    ///////////////////////////////

    if (receipt->src_url != NULL) {
        free(receipt->src_url);
        receipt->src_url = NULL;
    }

    if (receipt->src_sha != NULL) {
        free(receipt->src_sha);
        receipt->src_sha = NULL;
    }

    ///////////////////////////////

    if (receipt->fix_url != NULL) {
        free(receipt->fix_url);
        receipt->fix_url = NULL;
    }

    if (receipt->fix_sha != NULL) {
        free(receipt->fix_sha);
        receipt->fix_sha = NULL;
    }

    ///////////////////////////////

    if (receipt->res_url != NULL) {
        free(receipt->res_url);
        receipt->res_url = NULL;
    }

    if (receipt->res_sha != NULL) {
        free(receipt->res_sha);
        receipt->res_sha = NULL;
    }

    ///////////////////////////////

    if (receipt->dep_pkg != NULL) {
        free(receipt->dep_pkg);
        receipt->dep_pkg = NULL;
    }

    if (receipt->dep_upp != NULL) {
        free(receipt->dep_upp);
        receipt->dep_upp = NULL;
    }

    if (receipt->dep_pym != NULL) {
        free(receipt->dep_pym);
        receipt->dep_pym = NULL;
    }

    if (receipt->dep_plm != NULL) {
        free(receipt->dep_plm);
        receipt->dep_plm = NULL;
    }

    ///////////////////////////////

    if (receipt->bsystem != NULL) {
        free(receipt->bsystem);
        receipt->bsystem = NULL;
    }

    if (receipt->bscript != NULL) {
        free(receipt->bscript);
        receipt->bscript = NULL;
    }

    ///////////////////////////////

    if (receipt->cdefine != NULL) {
        free(receipt->cdefine);
        receipt->cdefine = NULL;
    }

    if (receipt->ccflags != NULL) {
        free(receipt->ccflags);
        receipt->ccflags = NULL;
    }

    if (receipt->xxflags != NULL) {
        free(receipt->xxflags);
        receipt->xxflags = NULL;
    }

    if (receipt->ldflags != NULL) {
        free(receipt->ldflags);
        receipt->ldflags = NULL;
    }

    ///////////////////////////////

    if (receipt->exetype != NULL) {
        free(receipt->exetype);
        receipt->exetype = NULL;
    }

    ///////////////////////////////

    if (receipt->prepare != NULL) {
        free(receipt->prepare);
        receipt->prepare = NULL;
    }

    if (receipt->install != NULL) {
        free(receipt->install);
        receipt->install = NULL;
    }

    ///////////////////////////////

    if (receipt->path != NULL) {
        free(receipt->path);
        receipt->path = NULL;
    }

    ///////////////////////////////

    if (receipt->signature != NULL) {
        free(receipt->signature);
        receipt->signature = NULL;
    }

    if (receipt->timestamp != NULL) {
        free(receipt->timestamp);
        receipt->timestamp = NULL;
    }

    free(receipt);
}

static PPKGReceiptKeyCode ppkg_receipt_key_code_from_key_name(char * key) {
           if (strcmp(key, "summary") == 0) {
        return RECEIPT_KEY_CODE_summary;
    } else if (strcmp(key, "version") == 0) {
        return RECEIPT_KEY_CODE_version;
    } else if (strcmp(key, "license") == 0) {
        return RECEIPT_KEY_CODE_license;
    } else if (strcmp(key, "web-url") == 0) {
        return RECEIPT_KEY_CODE_web_url;
    } else if (strcmp(key, "git-url") == 0) {
        return RECEIPT_KEY_CODE_git_url;
    } else if (strcmp(key, "git-sha") == 0) {
        return RECEIPT_KEY_CODE_git_sha;
    } else if (strcmp(key, "git-ref") == 0) {
        return RECEIPT_KEY_CODE_git_ref;
    } else if (strcmp(key, "shallow") == 0) {
        return RECEIPT_KEY_CODE_shallow;
    } else if (strcmp(key, "src-url") == 0) {
        return RECEIPT_KEY_CODE_src_url;
    } else if (strcmp(key, "src-sha") == 0) {
        return RECEIPT_KEY_CODE_src_sha;
    } else if (strcmp(key, "fix-url") == 0) {
        return RECEIPT_KEY_CODE_fix_url;
    } else if (strcmp(key, "fix-sha") == 0) {
        return RECEIPT_KEY_CODE_fix_sha;
    } else if (strcmp(key, "res-url") == 0) {
        return RECEIPT_KEY_CODE_res_url;
    } else if (strcmp(key, "res-sha") == 0) {
        return RECEIPT_KEY_CODE_res_sha;
    } else if (strcmp(key, "dep-pkg") == 0) {
        return RECEIPT_KEY_CODE_dep_pkg;
    } else if (strcmp(key, "dep-upp") == 0) {
        return RECEIPT_KEY_CODE_dep_upp;
    } else if (strcmp(key, "dep-pym") == 0) {
        return RECEIPT_KEY_CODE_dep_pym;
    } else if (strcmp(key, "dep-plm") == 0) {
        return RECEIPT_KEY_CODE_dep_plm;
    } else if (strcmp(key, "cdefine") == 0) {
        return RECEIPT_KEY_CODE_cdefine;
    } else if (strcmp(key, "ccflags") == 0) {
        return RECEIPT_KEY_CODE_ccflags;
    } else if (strcmp(key, "xxflags") == 0) {
        return RECEIPT_KEY_CODE_xxflags;
    } else if (strcmp(key, "ldflags") == 0) {
        return RECEIPT_KEY_CODE_ldflags;
    } else if (strcmp(key, "prepare") == 0) {
        return RECEIPT_KEY_CODE_prepare;
    } else if (strcmp(key, "install") == 0) {
        return RECEIPT_KEY_CODE_install;
    } else if (strcmp(key, "symlink") == 0) {
        return RECEIPT_KEY_CODE_symlink;
    } else if (strcmp(key, "bsystem") == 0) {
        return RECEIPT_KEY_CODE_bsystem;
    } else if (strcmp(key, "bscript") == 0) {
        return RECEIPT_KEY_CODE_bscript;
    } else if (strcmp(key, "binbstd") == 0) {
        return RECEIPT_KEY_CODE_binbstd;
    } else if (strcmp(key, "exetype") == 0) {
        return RECEIPT_KEY_CODE_exetype;
    } else if (strcmp(key, "parallel") == 0) {
        return RECEIPT_KEY_CODE_parallel;
    } else if (strcmp(key, "signature") == 0) {
        return RECEIPT_KEY_CODE_signature;
    } else if (strcmp(key, "timestamp") == 0) {
        return RECEIPT_KEY_CODE_timestamp;
    } else {
        return RECEIPT_KEY_CODE_unknown;
    }
}

static void ppkg_receipt_set_value(PPKGReceiptKeyCode keyCode, char * value, PPKGReceipt * receipt) {
    switch (keyCode) {
        case RECEIPT_KEY_CODE_summary: if (receipt->summary != NULL) free(receipt->summary); receipt->summary = strdup(value); break;
        case RECEIPT_KEY_CODE_version: if (receipt->version != NULL) free(receipt->version); receipt->version = strdup(value); break;
        case RECEIPT_KEY_CODE_license: if (receipt->license != NULL) free(receipt->license); receipt->license = strdup(value); break;

        case RECEIPT_KEY_CODE_web_url: if (receipt->web_url != NULL) free(receipt->web_url); receipt->web_url = strdup(value); break;

        case RECEIPT_KEY_CODE_git_url: if (receipt->git_url != NULL) free(receipt->git_url); receipt->git_url = strdup(value); break;
        case RECEIPT_KEY_CODE_git_sha: if (receipt->git_sha != NULL) free(receipt->git_sha); receipt->git_sha = strdup(value); break;
        case RECEIPT_KEY_CODE_git_ref: if (receipt->git_ref != NULL) free(receipt->git_ref); receipt->git_ref = strdup(value); break;

        case RECEIPT_KEY_CODE_src_url: if (receipt->src_url != NULL) free(receipt->src_url); receipt->src_url = strdup(value); break;
        case RECEIPT_KEY_CODE_src_sha: if (receipt->src_sha != NULL) free(receipt->src_sha); receipt->src_sha = strdup(value); break;

        case RECEIPT_KEY_CODE_fix_url: if (receipt->fix_url != NULL) free(receipt->fix_url); receipt->fix_url = strdup(value); break;
        case RECEIPT_KEY_CODE_fix_sha: if (receipt->fix_sha != NULL) free(receipt->fix_sha); receipt->fix_sha = strdup(value); break;

        case RECEIPT_KEY_CODE_res_url: if (receipt->res_url != NULL) free(receipt->res_url); receipt->res_url = strdup(value); break;
        case RECEIPT_KEY_CODE_res_sha: if (receipt->res_sha != NULL) free(receipt->res_sha); receipt->res_sha = strdup(value); break;

        case RECEIPT_KEY_CODE_dep_pkg: if (receipt->dep_pkg != NULL) free(receipt->dep_pkg); receipt->dep_pkg = strdup(value); break;
        case RECEIPT_KEY_CODE_dep_upp: if (receipt->dep_upp != NULL) free(receipt->dep_upp); receipt->dep_upp = strdup(value); break;
        case RECEIPT_KEY_CODE_dep_pym: if (receipt->dep_pym != NULL) free(receipt->dep_pym); receipt->dep_pym = strdup(value); break;
        case RECEIPT_KEY_CODE_dep_plm: if (receipt->dep_plm != NULL) free(receipt->dep_plm); receipt->dep_plm = strdup(value); break;

        case RECEIPT_KEY_CODE_cdefine: if (receipt->cdefine != NULL) free(receipt->cdefine); receipt->cdefine = strdup(value); break;
        case RECEIPT_KEY_CODE_ccflags: if (receipt->ccflags != NULL) free(receipt->ccflags); receipt->ccflags = strdup(value); break;
        case RECEIPT_KEY_CODE_xxflags: if (receipt->xxflags != NULL) free(receipt->xxflags); receipt->xxflags = strdup(value); break;
        case RECEIPT_KEY_CODE_ldflags: if (receipt->ldflags != NULL) free(receipt->ldflags); receipt->ldflags = strdup(value); break;

        case RECEIPT_KEY_CODE_prepare: if (receipt->prepare != NULL) free(receipt->prepare); receipt->prepare = strdup(value); break;
        case RECEIPT_KEY_CODE_install: if (receipt->install != NULL) free(receipt->install); receipt->install = strdup(value); break;

        case RECEIPT_KEY_CODE_exetype: if (receipt->exetype != NULL) free(receipt->exetype); receipt->exetype = strdup(value); break;

        case RECEIPT_KEY_CODE_bsystem: if (receipt->bsystem != NULL) free(receipt->bsystem); receipt->bsystem = strdup(value); break;
        case RECEIPT_KEY_CODE_bscript: if (receipt->bscript != NULL) free(receipt->bscript); receipt->bscript = strdup(value); break;

        case RECEIPT_KEY_CODE_signature: if (receipt->signature != NULL) free(receipt->signature); receipt->signature = strdup(value); break;
        case RECEIPT_KEY_CODE_timestamp: if (receipt->timestamp != NULL) free(receipt->timestamp); receipt->timestamp = strdup(value); break;

        case RECEIPT_KEY_CODE_shallow:
            if (value == NULL) {
                receipt->shallow = false;
            }
            if (strcmp(value, "yes") == 0) {
                receipt->shallow = true;
            }
            break;

        case RECEIPT_KEY_CODE_binbstd:
            if (value == NULL) {
                receipt->binbstd = false;
            }
            if (strcmp(value, "yes") == 0) {
                receipt->binbstd = true;
            }
            break;

        case RECEIPT_KEY_CODE_symlink:
            if (value == NULL) {
                receipt->symlink = false;
            }
            if (strcmp(value, "yes") == 0) {
                receipt->symlink = true;
            }
            break;

        case RECEIPT_KEY_CODE_parallel:
            if (value == NULL) {
                receipt->parallel = false;
            }
            if (strcmp(value, "yes") == 0) {
                receipt->parallel = true;
            }
            break;

        default: break;
    }
}

static int ppkg_receipt_check(PPKGReceipt * receipt, const char * receiptFilePath) {
    if (receipt->summary == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : summary mapping not found.\n", receiptFilePath);
        return PPKG_FORMULA_SCHEME_ERROR;
    }

    if (strcmp(receipt->summary, "") == 0) {
        fprintf(stderr, "scheme error in receipt file: %s : summary mapping's value must not be empty.\n", receiptFilePath);
        return PPKG_FORMULA_SCHEME_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (receipt->version == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : version mapping not found.\n", receiptFilePath);
        return PPKG_FORMULA_SCHEME_ERROR;
    }

    if (strcmp(receipt->version, "") == 0) {
        fprintf(stderr, "scheme error in receipt file: %s : version mapping's value must not be empty.\n", receiptFilePath);
        return PPKG_FORMULA_SCHEME_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (receipt->web_url == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : web-url mapping not found.\n", receiptFilePath);
        return PPKG_FORMULA_SCHEME_ERROR;
    }

    if (strcmp(receipt->web_url, "") == 0) {
        fprintf(stderr, "scheme error in receipt file: %s : web-url mapping's value must not be empty.\n", receiptFilePath);
        return PPKG_FORMULA_SCHEME_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (receipt->src_url == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : src-url mapping not found.\n", receiptFilePath);
        return PPKG_FORMULA_SCHEME_ERROR;
    }

    if (strcmp(receipt->src_url, "") == 0) {
        fprintf(stderr, "scheme error in receipt file: %s : src-url mapping's value must not be empty.\n", receiptFilePath);
        return PPKG_FORMULA_SCHEME_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (receipt->src_sha == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : src-sha mapping not found.\n", receiptFilePath);
        return PPKG_FORMULA_SCHEME_ERROR;
    }

    if (strlen(receipt->src_sha) != 64) {
        fprintf(stderr, "scheme error in receipt file: %s : src-sha mapping's value's length must be 64.\n", receiptFilePath);
        return PPKG_FORMULA_SCHEME_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (receipt->timestamp == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : timestamp mapping not found.\n", receiptFilePath);
        return PPKG_RECEIPT_SCHEME_ERROR;
    }

    size_t i = 0;
    char   c;

    for (;; i++) {
        c = receipt->timestamp[i];

        if (c == '\0') {
            break;
        }

        if ((c < '0') || (c > '9')) {
            fprintf(stderr, "scheme error in receipt file: %s : timestamp mapping's value should only contains non-numeric characters.\n", receiptFilePath);
            return PPKG_RECEIPT_SCHEME_ERROR;
        }
    }

    if (i != 10) {
        fprintf(stderr, "scheme error in receipt file: %s : timestamp mapping's value's length must be 10.\n", receiptFilePath);
        return PPKG_RECEIPT_SCHEME_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (receipt->signature == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : signature mapping not found.\n", receiptFilePath);
        return PPKG_RECEIPT_SCHEME_ERROR;
    }

    if (strcmp(receipt->signature, "") == 0) {
        fprintf(stderr, "scheme error in receipt file: %s : signature mapping's value must not be empty.\n", receiptFilePath);
        return PPKG_RECEIPT_SCHEME_ERROR;
    }


    return PPKG_OK;
}

int ppkg_receipt_parse(const char * packageName, PPKGReceipt * * out) {
    int resultCode = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    char * receiptFilePath = (char*)calloc(userHomeDirLength + strlen(packageName) + 36, sizeof(char));

    if (receiptFilePath == NULL) {
        return PPKG_ERROR_ALLOCATE_MEMORY_FAILED;
    }

    sprintf(receiptFilePath, "%s/.ppkg/installed/%s/.ppkg/receipt.yml", userHomeDir, packageName);

    FILE * file = fopen(receiptFilePath, "r");

    if (file == NULL) {
        perror(receiptFilePath);
        free(receiptFilePath);
        return PPKG_PACKAGE_IS_NOT_INSTALLED;
    }

    yaml_parser_t parser;
    yaml_token_t  token;

    // https://libyaml.docsforge.com/master/api/yaml_parser_initialize/
    if (yaml_parser_initialize(&parser) == 0) {
        perror("Failed to initialize yaml parser");
        free(receiptFilePath);
        return PPKG_ERROR;
    }

    yaml_parser_set_input_file(&parser, file);

    PPKGReceiptKeyCode receiptKeyCode = RECEIPT_KEY_CODE_unknown;

    PPKGReceipt * receipt = NULL;

    int lastTokenType = 0;

    do {
        // https://libyaml.docsforge.com/master/api/yaml_parser_scan/
        if (yaml_parser_scan(&parser, &token) == 0) {
            fprintf(stderr, "syntax error in receipt file: %s\n", receiptFilePath);
            resultCode = PPKG_RECEIPT_SYNTAX_ERROR;
            goto clean;
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
                    receiptKeyCode = ppkg_receipt_key_code_from_key_name((char*)token.data.scalar.value);
                } else if (lastTokenType == 2) {
                    if (receipt == NULL) {
                        receipt = (PPKGReceipt*)calloc(1, sizeof(PPKGReceipt));

                        if (receipt == NULL) {
                            resultCode = PPKG_ERROR_ALLOCATE_MEMORY_FAILED;
                            goto clean;
                        }

                        receipt->path = receiptFilePath;
                        receipt->shallow = true;
                        receipt->symlink = true;
                        receipt->binbstd = false;
                        receipt->parallel = true;
                    }

                    ppkg_receipt_set_value(receiptKeyCode, (char*)token.data.scalar.value, receipt);
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

clean:
    yaml_token_delete(&token);

    yaml_parser_delete(&parser);

    fclose(file);

    //ppkg_receipt_dump(receipt);

    if (resultCode == PPKG_OK) {
        resultCode = ppkg_receipt_check(receipt, receiptFilePath);

        if (resultCode == PPKG_OK) {
            (*out) = receipt;
            return PPKG_OK;
        }
    }

    if (receipt == NULL) {
        free(receiptFilePath);
    } else {
        ppkg_receipt_free(receipt);
    }

    return resultCode;
}
