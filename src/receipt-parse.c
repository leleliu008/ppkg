#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <limits.h>

#include <yaml.h>

#include "core/regex/regex.h"

#include "ppkg.h"

typedef enum {
    PPKGReceiptKeyCode_unknown,

    PPKGReceiptKeyCode_summary,
    PPKGReceiptKeyCode_version,
    PPKGReceiptKeyCode_license,

    PPKGReceiptKeyCode_web_url,

    PPKGReceiptKeyCode_git_url,
    PPKGReceiptKeyCode_git_sha,
    PPKGReceiptKeyCode_git_ref,
    PPKGReceiptKeyCode_git_nth,

    PPKGReceiptKeyCode_src_url,
    PPKGReceiptKeyCode_src_uri,
    PPKGReceiptKeyCode_src_sha,

    PPKGReceiptKeyCode_fix_url,
    PPKGReceiptKeyCode_fix_uri,
    PPKGReceiptKeyCode_fix_sha,

    PPKGReceiptKeyCode_res_url,
    PPKGReceiptKeyCode_res_uri,
    PPKGReceiptKeyCode_res_sha,

    PPKGReceiptKeyCode_dep_pkg,
    PPKGReceiptKeyCode_dep_upp,
    PPKGReceiptKeyCode_dep_pym,
    PPKGReceiptKeyCode_dep_plm,

    PPKGReceiptKeyCode_bsystem,
    PPKGReceiptKeyCode_bscript,
    PPKGReceiptKeyCode_binbstd,

    PPKGReceiptKeyCode_dopatch,
    PPKGReceiptKeyCode_install,
    PPKGReceiptKeyCode_symlink,

    PPKGReceiptKeyCode_ppflags,
    PPKGReceiptKeyCode_ccflags,
    PPKGReceiptKeyCode_xxflags,
    PPKGReceiptKeyCode_ldflags,

    PPKGReceiptKeyCode_parallel,

    PPKGReceiptKeyCode_signature,
    PPKGReceiptKeyCode_timestamp,
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
    printf("git_nth: %zu\n", receipt->git_nth);

    printf("src_url: %s\n", receipt->src_url);
    printf("src_uri: %s\n", receipt->src_uri);
    printf("src_sha: %s\n", receipt->src_sha);

    printf("fix_url: %s\n", receipt->fix_url);
    printf("fix_uri: %s\n", receipt->fix_uri);
    printf("fix_sha: %s\n", receipt->fix_sha);

    printf("res_url: %s\n", receipt->res_url);
    printf("res_uri: %s\n", receipt->res_uri);
    printf("res_sha: %s\n", receipt->res_sha);

    printf("dep_pkg: %s\n", receipt->dep_pkg);
    printf("dep_upp: %s\n", receipt->dep_upp);
    printf("dep_pym: %s\n", receipt->dep_pym);
    printf("dep_plm: %s\n", receipt->dep_plm);

    printf("bsystem: %s\n", receipt->bsystem);
    printf("bscript: %s\n", receipt->bscript);
    printf("binbstd: %d\n", receipt->binbstd);
    printf("parallel: %d\n", receipt->parallel);

    printf("dopatch: %s\n", receipt->dopatch);
    printf("install: %s\n", receipt->install);

    printf("symlink: %d\n", receipt->symlink);

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

    if (receipt->src_uri != NULL) {
        free(receipt->src_uri);
        receipt->src_uri = NULL;
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

    if (receipt->fix_uri != NULL) {
        free(receipt->fix_uri);
        receipt->fix_uri = NULL;
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

    if (receipt->res_uri != NULL) {
        free(receipt->res_uri);
        receipt->res_uri = NULL;
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

    if (receipt->ppflags != NULL) {
        free(receipt->ppflags);
        receipt->ppflags = NULL;
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

    if (receipt->dopatch != NULL) {
        free(receipt->dopatch);
        receipt->dopatch = NULL;
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
        return PPKGReceiptKeyCode_summary;
    } else if (strcmp(key, "version") == 0) {
        return PPKGReceiptKeyCode_version;
    } else if (strcmp(key, "license") == 0) {
        return PPKGReceiptKeyCode_license;
    } else if (strcmp(key, "web-url") == 0) {
        return PPKGReceiptKeyCode_web_url;
    } else if (strcmp(key, "git-url") == 0) {
        return PPKGReceiptKeyCode_git_url;
    } else if (strcmp(key, "git-sha") == 0) {
        return PPKGReceiptKeyCode_git_sha;
    } else if (strcmp(key, "git-ref") == 0) {
        return PPKGReceiptKeyCode_git_ref;
    } else if (strcmp(key, "git-nth") == 0) {
        return PPKGReceiptKeyCode_git_nth;
    } else if (strcmp(key, "src-url") == 0) {
        return PPKGReceiptKeyCode_src_url;
    } else if (strcmp(key, "src-uri") == 0) {
        return PPKGReceiptKeyCode_src_uri;
    } else if (strcmp(key, "src-sha") == 0) {
        return PPKGReceiptKeyCode_src_sha;
    } else if (strcmp(key, "fix-url") == 0) {
        return PPKGReceiptKeyCode_fix_url;
    } else if (strcmp(key, "fix-uri") == 0) {
        return PPKGReceiptKeyCode_fix_uri;
    } else if (strcmp(key, "fix-sha") == 0) {
        return PPKGReceiptKeyCode_fix_sha;
    } else if (strcmp(key, "res-url") == 0) {
        return PPKGReceiptKeyCode_res_url;
    } else if (strcmp(key, "res-uri") == 0) {
        return PPKGReceiptKeyCode_res_uri;
    } else if (strcmp(key, "res-sha") == 0) {
        return PPKGReceiptKeyCode_res_sha;
    } else if (strcmp(key, "dep-pkg") == 0) {
        return PPKGReceiptKeyCode_dep_pkg;
    } else if (strcmp(key, "dep-upp") == 0) {
        return PPKGReceiptKeyCode_dep_upp;
    } else if (strcmp(key, "dep-pym") == 0) {
        return PPKGReceiptKeyCode_dep_pym;
    } else if (strcmp(key, "dep-plm") == 0) {
        return PPKGReceiptKeyCode_dep_plm;
    } else if (strcmp(key, "ppflags") == 0) {
        return PPKGReceiptKeyCode_ppflags;
    } else if (strcmp(key, "ccflags") == 0) {
        return PPKGReceiptKeyCode_ccflags;
    } else if (strcmp(key, "xxflags") == 0) {
        return PPKGReceiptKeyCode_xxflags;
    } else if (strcmp(key, "ldflags") == 0) {
        return PPKGReceiptKeyCode_ldflags;
    } else if (strcmp(key, "dopatch") == 0) {
        return PPKGReceiptKeyCode_dopatch;
    } else if (strcmp(key, "install") == 0) {
        return PPKGReceiptKeyCode_install;
    } else if (strcmp(key, "symlink") == 0) {
        return PPKGReceiptKeyCode_symlink;
    } else if (strcmp(key, "bsystem") == 0) {
        return PPKGReceiptKeyCode_bsystem;
    } else if (strcmp(key, "bscript") == 0) {
        return PPKGReceiptKeyCode_bscript;
    } else if (strcmp(key, "binbstd") == 0) {
        return PPKGReceiptKeyCode_binbstd;
    } else if (strcmp(key, "parallel") == 0) {
        return PPKGReceiptKeyCode_parallel;
    } else if (strcmp(key, "signature") == 0) {
        return PPKGReceiptKeyCode_signature;
    } else if (strcmp(key, "timestamp") == 0) {
        return PPKGReceiptKeyCode_timestamp;
    } else {
        return PPKGReceiptKeyCode_unknown;
    }
}

static void ppkg_receipt_set_value(PPKGReceiptKeyCode keyCode, char * value, PPKGReceipt * receipt) {
    if (keyCode == PPKGReceiptKeyCode_unknown) {
        return;
    }

    for (;;) {
        char c = value[0];

        if (c == '\0') {
            return;
        }

        // non-printable ASCII characters and space
        if (c <= 32) {
            value++;
        } else {
            break;
        }
    }

    switch (keyCode) {
        case PPKGReceiptKeyCode_summary: if (receipt->summary != NULL) free(receipt->summary); receipt->summary = strdup(value); break;
        case PPKGReceiptKeyCode_version: if (receipt->version != NULL) free(receipt->version); receipt->version = strdup(value); break;
        case PPKGReceiptKeyCode_license: if (receipt->license != NULL) free(receipt->license); receipt->license = strdup(value); break;

        case PPKGReceiptKeyCode_web_url: if (receipt->web_url != NULL) free(receipt->web_url); receipt->web_url = strdup(value); break;

        case PPKGReceiptKeyCode_git_url: if (receipt->git_url != NULL) free(receipt->git_url); receipt->git_url = strdup(value); break;
        case PPKGReceiptKeyCode_git_sha: if (receipt->git_sha != NULL) free(receipt->git_sha); receipt->git_sha = strdup(value); break;
        case PPKGReceiptKeyCode_git_ref: if (receipt->git_ref != NULL) free(receipt->git_ref); receipt->git_ref = strdup(value); break;

        case PPKGReceiptKeyCode_src_url: if (receipt->src_url != NULL) free(receipt->src_url); receipt->src_url = strdup(value); break;
        case PPKGReceiptKeyCode_src_uri: if (receipt->src_uri != NULL) free(receipt->src_uri); receipt->src_uri = strdup(value); break;
        case PPKGReceiptKeyCode_src_sha: if (receipt->src_sha != NULL) free(receipt->src_sha); receipt->src_sha = strdup(value); break;

        case PPKGReceiptKeyCode_fix_url: if (receipt->fix_url != NULL) free(receipt->fix_url); receipt->fix_url = strdup(value); break;
        case PPKGReceiptKeyCode_fix_uri: if (receipt->fix_uri != NULL) free(receipt->fix_uri); receipt->fix_uri = strdup(value); break;
        case PPKGReceiptKeyCode_fix_sha: if (receipt->fix_sha != NULL) free(receipt->fix_sha); receipt->fix_sha = strdup(value); break;

        case PPKGReceiptKeyCode_res_url: if (receipt->res_url != NULL) free(receipt->res_url); receipt->res_url = strdup(value); break;
        case PPKGReceiptKeyCode_res_uri: if (receipt->res_uri != NULL) free(receipt->res_uri); receipt->res_uri = strdup(value); break;
        case PPKGReceiptKeyCode_res_sha: if (receipt->res_sha != NULL) free(receipt->res_sha); receipt->res_sha = strdup(value); break;

        case PPKGReceiptKeyCode_dep_pkg: if (receipt->dep_pkg != NULL) free(receipt->dep_pkg); receipt->dep_pkg = strdup(value); break;
        case PPKGReceiptKeyCode_dep_upp: if (receipt->dep_upp != NULL) free(receipt->dep_upp); receipt->dep_upp = strdup(value); break;
        case PPKGReceiptKeyCode_dep_pym: if (receipt->dep_pym != NULL) free(receipt->dep_pym); receipt->dep_pym = strdup(value); break;
        case PPKGReceiptKeyCode_dep_plm: if (receipt->dep_plm != NULL) free(receipt->dep_plm); receipt->dep_plm = strdup(value); break;

        case PPKGReceiptKeyCode_ppflags: if (receipt->ppflags != NULL) free(receipt->ppflags); receipt->ppflags = strdup(value); break;
        case PPKGReceiptKeyCode_ccflags: if (receipt->ccflags != NULL) free(receipt->ccflags); receipt->ccflags = strdup(value); break;
        case PPKGReceiptKeyCode_xxflags: if (receipt->xxflags != NULL) free(receipt->xxflags); receipt->xxflags = strdup(value); break;
        case PPKGReceiptKeyCode_ldflags: if (receipt->ldflags != NULL) free(receipt->ldflags); receipt->ldflags = strdup(value); break;

        case PPKGReceiptKeyCode_dopatch: if (receipt->dopatch != NULL) free(receipt->dopatch); receipt->dopatch = strdup(value); break;
        case PPKGReceiptKeyCode_install: if (receipt->install != NULL) free(receipt->install); receipt->install = strdup(value); break;

        case PPKGReceiptKeyCode_bsystem: if (receipt->bsystem != NULL) free(receipt->bsystem); receipt->bsystem = strdup(value); break;
        case PPKGReceiptKeyCode_bscript: if (receipt->bscript != NULL) free(receipt->bscript); receipt->bscript = strdup(value); break;

        case PPKGReceiptKeyCode_signature: if (receipt->signature != NULL) free(receipt->signature); receipt->signature = strdup(value); break;
        case PPKGReceiptKeyCode_timestamp: if (receipt->timestamp != NULL) free(receipt->timestamp); receipt->timestamp = strdup(value); break;

        case PPKGReceiptKeyCode_git_nth:
            receipt->git_nth = atoi(value);
            break;

        case PPKGReceiptKeyCode_binbstd:
            if (strcmp(value, "yes") == 0) {
                receipt->binbstd = true;
            }
            break;

        case PPKGReceiptKeyCode_symlink:
            if (strcmp(value, "yes") == 0) {
                receipt->symlink = true;
            }
            break;

        case PPKGReceiptKeyCode_parallel:
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
        return PPKG_ERROR_FORMULA_SCHEME;
    }

    if (receipt->version == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : version mapping not found.\n", receiptFilePath);
        return PPKG_ERROR_FORMULA_SCHEME;
    }

    if (receipt->web_url == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : web-url mapping not found.\n", receiptFilePath);
        return PPKG_ERROR_FORMULA_SCHEME;
    }

    if (receipt->src_url == NULL) {
        if (receipt->git_url == NULL) {
            fprintf(stderr, "scheme error in receipt file: %s : neither src-url nor git-url mapping not found.\n", receiptFilePath);
            return PPKG_ERROR_FORMULA_SCHEME;
        }
    } else {
        if (strncmp(receipt->src_url, "dir://", 6) == 0) {
            ;
        } else {
            if (receipt->src_sha == NULL) {
                fprintf(stderr, "scheme error in receipt file: %s : src-sha mapping not found.\n", receiptFilePath);
                return PPKG_ERROR_FORMULA_SCHEME;
            }

            if (strlen(receipt->src_sha) != 64) {
                fprintf(stderr, "scheme error in receipt file: %s : src-sha mapping's value's length must be 64.\n", receiptFilePath);
                return PPKG_ERROR_FORMULA_SCHEME;
            }
        }
    }

    if (receipt->bsystem == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : bsystem mapping not found.\n", receiptFilePath);
        return PPKG_ERROR_FORMULA_SCHEME;
    }

    if (receipt->signature == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : signature mapping not found.\n", receiptFilePath);
        return PPKG_ERROR_RECEIPT_SCHEME;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (receipt->timestamp == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : timestamp mapping not found.\n", receiptFilePath);
        return PPKG_ERROR_RECEIPT_SCHEME;
    }

    size_t i = 0;

    for (;; i++) {
        char c = receipt->timestamp[i];

        if (c == '\0') {
            break;
        }

        if ((c < '0') || (c > '9')) {
            fprintf(stderr, "scheme error in receipt file: %s : timestamp mapping's value should only contains non-numeric characters.\n", receiptFilePath);
            return PPKG_ERROR_RECEIPT_SCHEME;
        }
    }

    if (i != 10U) {
        fprintf(stderr, "scheme error in receipt file: %s : timestamp mapping's value's length must be 10.\n", receiptFilePath);
        return PPKG_ERROR_RECEIPT_SCHEME;
    }

    return PPKG_OK;
}

int ppkg_receipt_parse(const char * packageName, PPKGReceipt * * out) {
    int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t receiptFilePathLength = ppkgHomeDIRLength + strlen(packageName) + 30U;
    char * receiptFilePath = (char*)calloc(receiptFilePathLength, sizeof(char));

    if (receiptFilePath == NULL) {
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    snprintf(receiptFilePath, receiptFilePathLength, "%s/installed/%s/.ppkg/RECEIPT.yml", ppkgHomeDIR, packageName);

    FILE * file = fopen(receiptFilePath, "r");

    if (file == NULL) {
        perror(receiptFilePath);
        free(receiptFilePath);
        return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
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

    PPKGReceiptKeyCode receiptKeyCode = PPKGReceiptKeyCode_unknown;

    PPKGReceipt * receipt = NULL;

    int lastTokenType = 0;

    do {
        // https://libyaml.docsforge.com/master/api/yaml_parser_scan/
        if (yaml_parser_scan(&parser, &token) == 0) {
            fprintf(stderr, "syntax error in receipt file: %s\n", receiptFilePath);
            ret = PPKG_ERROR_RECEIPT_SYNTAX;
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
                    receiptKeyCode = ppkg_receipt_key_code_from_key_name((char*)token.data.scalar.value);
                } else if (lastTokenType == 2) {
                    if (receipt == NULL) {
                        receipt = (PPKGReceipt*)calloc(1, sizeof(PPKGReceipt));

                        if (receipt == NULL) {
                            ret = PPKG_ERROR_MEMORY_ALLOCATE;
                            goto finalize;
                        }

                        receipt->path = receiptFilePath;
                        receipt->git_nth = 1U;
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

finalize:
    yaml_token_delete(&token);

    yaml_parser_delete(&parser);

    fclose(file);

    //ppkg_receipt_dump(receipt);

    if (ret == PPKG_OK) {
        ret = ppkg_receipt_check(receipt, receiptFilePath);

        if (ret == PPKG_OK) {
            (*out) = receipt;
            return PPKG_OK;
        }
    }

    if (receipt == NULL) {
        free(receiptFilePath);
    } else {
        ppkg_receipt_free(receipt);
    }

    return ret;
}
