#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <limits.h>
#include <sys/stat.h>

#include <yaml.h>

#include "uppm.h"
#include "ppkg.h"

typedef enum {
    UPPMReceiptKeyCode_unknown,
    UPPMReceiptKeyCode_summary,
    UPPMReceiptKeyCode_version,
    UPPMReceiptKeyCode_license,
    UPPMReceiptKeyCode_webpage,
    UPPMReceiptKeyCode_bin_url,
    UPPMReceiptKeyCode_bin_sha,
    UPPMReceiptKeyCode_dep_pkg,
    UPPMReceiptKeyCode_install,
    UPPMReceiptKeyCode_timestamp,
    UPPMReceiptKeyCode_signature,
} UPPMReceiptKeyCode;

void uppm_receipt_dump(UPPMReceipt * receipt) {
    if (receipt == NULL) {
        return;
    }

    printf("summary: %s\n", receipt->summary);
    printf("version: %s\n", receipt->version);
    printf("license: %s\n", receipt->license);
    printf("webpage: %s\n", receipt->webpage);
    printf("bin-url: %s\n", receipt->bin_url);
    printf("bin-sha: %s\n", receipt->bin_sha);
    printf("dep-pkg: %s\n", receipt->dep_pkg);
    printf("install: %s\n", receipt->install);
    printf("signature: %s\n", receipt->signature);
    printf("timestamp: %s\n", receipt->timestamp);
}

void uppm_receipt_free(UPPMReceipt * receipt) {
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

    if (receipt->webpage != NULL) {
        free(receipt->webpage);
        receipt->webpage = NULL;
    }

    if (receipt->bin_url != NULL) {
        free(receipt->bin_url);
        receipt->bin_url = NULL;
    }

    if (receipt->bin_sha != NULL) {
        free(receipt->bin_sha);
        receipt->bin_sha = NULL;
    }

    if (receipt->dep_pkg != NULL) {
        free(receipt->dep_pkg);
        receipt->dep_pkg = NULL;
    }

    if (receipt->install != NULL) {
        free(receipt->install);
        receipt->install = NULL;
    }

    if (receipt->timestamp != NULL) {
        free(receipt->timestamp);
        receipt->timestamp = NULL;
    }

    if (receipt->signature != NULL) {
        free(receipt->signature);
        receipt->signature = NULL;
    }

    free(receipt);
}

static UPPMReceiptKeyCode uppm_receipt_key_code_from_key_name(char * key) {
           if (strcmp(key, "summary") == 0) {
        return UPPMReceiptKeyCode_summary;
    } else if (strcmp(key, "webpage") == 0) {
        return UPPMReceiptKeyCode_webpage;
    } else if (strcmp(key, "version") == 0) {
        return UPPMReceiptKeyCode_version;
    } else if (strcmp(key, "license") == 0) {
        return UPPMReceiptKeyCode_license;
    } else if (strcmp(key, "bin-url") == 0) {
        return UPPMReceiptKeyCode_bin_url;
    } else if (strcmp(key, "bin-sha") == 0) {
        return UPPMReceiptKeyCode_bin_sha;
    } else if (strcmp(key, "dep-pkg") == 0) {
        return UPPMReceiptKeyCode_dep_pkg;
    } else if (strcmp(key, "install") == 0) {
        return UPPMReceiptKeyCode_install;
    } else if (strcmp(key, "timestamp") == 0) {
        return UPPMReceiptKeyCode_timestamp;
    } else if (strcmp(key, "signature") == 0) {
        return UPPMReceiptKeyCode_signature;
    } else {
        return UPPMReceiptKeyCode_unknown;
    }
}

static int uppm_receipt_set_value(UPPMReceiptKeyCode keyCode, char * value, UPPMReceipt * receipt) {
    if (keyCode == UPPMReceiptKeyCode_unknown) {
        return PPKG_OK;
    }

    for (;;) {
        if (value[0] == '\0') {
            return PPKG_OK;
        }

        // non-printable ASCII characters and space
        if (value[0] <= 32) {
            value++;
        } else {
            break;
        }
    }

    switch (keyCode) {
        case UPPMReceiptKeyCode_summary:
            if (receipt->summary != NULL) {
                free(receipt->summary);
            }

            receipt->summary = strdup(value);

            if (receipt->summary == NULL) {
                return PPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return PPKG_OK;
            }
        case UPPMReceiptKeyCode_version:
            if (receipt->version != NULL) {
                free(receipt->version);
            }

            receipt->version = strdup(value);

            if (receipt->version == NULL) {
                return PPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return PPKG_OK;
            }
        case UPPMReceiptKeyCode_license:
            if (receipt->license != NULL) {
                free(receipt->license);
            }

            receipt->license = strdup(value);

            if (receipt->license == NULL) {
                return PPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return PPKG_OK;
            }
        case UPPMReceiptKeyCode_webpage:
            if (receipt->webpage != NULL) {
                free(receipt->webpage);
            }

            receipt->webpage = strdup(value);

            if (receipt->webpage == NULL) {
                return PPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return PPKG_OK;
            }
        case UPPMReceiptKeyCode_bin_url:
            if (receipt->bin_url != NULL) {
                free(receipt->bin_url);
            }

            receipt->bin_url = strdup(value);

            if (receipt->bin_url == NULL) {
                return PPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return PPKG_OK;
            }
        case UPPMReceiptKeyCode_bin_sha:
            if (receipt->bin_sha != NULL) {
                free(receipt->bin_sha);
            }

            receipt->bin_sha = strdup(value);

            if (receipt->bin_sha == NULL) {
                return PPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return PPKG_OK;
            }
        case UPPMReceiptKeyCode_dep_pkg:
            if (receipt->dep_pkg != NULL) {
                free(receipt->dep_pkg);
            }

            receipt->dep_pkg = strdup(value);

            if (receipt->dep_pkg == NULL) {
                return PPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return PPKG_OK;
            }
        case UPPMReceiptKeyCode_install:
            if (receipt->install != NULL) {
                free(receipt->install);
            }

            receipt->install = strdup(value);

            if (receipt->install == NULL) {
                return PPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return PPKG_OK;
            }
        case UPPMReceiptKeyCode_timestamp:
            if (receipt->timestamp != NULL) {
                free(receipt->timestamp);
            }

            receipt->timestamp = strdup(value);

            if (receipt->timestamp == NULL) {
                return PPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return PPKG_OK;
            }
        case UPPMReceiptKeyCode_signature:
            if (receipt->signature != NULL) {
                free(receipt->signature);
            }

            receipt->signature = strdup(value);

            if (receipt->signature == NULL) {
                return PPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return PPKG_OK;
            }
        default: return PPKG_OK;
    }
}

static int uppm_receipt_check(UPPMReceipt * receipt, const char * receiptFilePath) {
    if (receipt->summary == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : summary mapping not found.\n", receiptFilePath);
        return PPKG_ERROR_RECEIPT_SCHEME;
    }

    if (receipt->version == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : version mapping not found.\n", receiptFilePath);
        return PPKG_ERROR_RECEIPT_SCHEME;
    }

    if (receipt->webpage == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : webpage mapping not found.\n", receiptFilePath);
        return PPKG_ERROR_RECEIPT_SCHEME;
    }

    if (receipt->bin_url == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : bin-url mapping not found.\n", receiptFilePath);
        return PPKG_ERROR_RECEIPT_SCHEME;
    }

    if (receipt->bin_sha == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : bin-sha mapping not found.\n", receiptFilePath);
        return PPKG_ERROR_RECEIPT_SCHEME;
    }

    if (strlen(receipt->bin_sha) != 64) {
        fprintf(stderr, "scheme error in receipt file: %s : bin-sha mapping's value's length must be 64.\n", receiptFilePath);
        return PPKG_ERROR_RECEIPT_SCHEME;
    }

    if (receipt->signature == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : signature mapping not found.\n", receiptFilePath);
        return PPKG_ERROR_RECEIPT_SCHEME;
    }

    if (receipt->timestamp == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : timestamp mapping not found.\n", receiptFilePath);
        return PPKG_ERROR_RECEIPT_SCHEME;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

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

    if (i != 10) {
        fprintf(stderr, "scheme error in receipt file: %s : timestamp mapping's value's length must be 10.\n", receiptFilePath);
        return PPKG_ERROR_RECEIPT_SCHEME;
    }

    return PPKG_OK;
}


int uppm_receipt_parse(const char * packageName, UPPMReceipt * * out) {
    int ret = uppm_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    char   uppmHomeDIR[PATH_MAX];
    size_t uppmHomeDIRLength;

    ret = uppm_home_dir(uppmHomeDIR, PATH_MAX, &uppmHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t packageInstalledDIRLength = uppmHomeDIRLength + strlen(packageName) + 12U;
    char   packageInstalledDIR[packageInstalledDIRLength];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRLength, "%s/installed/%s", uppmHomeDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    struct stat st;

    if (stat(packageInstalledDIR, &st) != 0) {
        return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    size_t receiptFilePathLength = packageInstalledDIRLength + 20U;
    char   receiptFilePath[receiptFilePathLength];

    ret = snprintf(receiptFilePath, receiptFilePathLength, "%s/.uppm/receipt.yml", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (stat(receiptFilePath, &st) != 0 || (!S_ISREG(st.st_mode))) {
        return PPKG_ERROR_PACKAGE_IS_BROKEN;
    }

    FILE * file = fopen(receiptFilePath, "r");

    if (file == NULL) {
        perror(receiptFilePath);
        return PPKG_ERROR;
    }

    yaml_parser_t parser;
    yaml_token_t  token;

    // https://libyaml.docsforge.com/master/api/yaml_parser_initialize/
    if (yaml_parser_initialize(&parser) == 0) {
        perror("Failed to initialize yaml parser");
        fclose(file);
        return PPKG_ERROR;
    }

    yaml_parser_set_input_file(&parser, file);

    UPPMReceiptKeyCode receiptKeyCode = UPPMReceiptKeyCode_unknown;

    UPPMReceipt * receipt = NULL;

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
                    receiptKeyCode = uppm_receipt_key_code_from_key_name((char*)token.data.scalar.value);
                } else if (lastTokenType == 2) {
                    if (receipt == NULL) {
                        receipt = (UPPMReceipt*)calloc(1, sizeof(UPPMReceipt));

                        if (receipt == NULL) {
                            ret = PPKG_ERROR_MEMORY_ALLOCATE;
                            goto finalize;
                        }
                    }

                    ret = uppm_receipt_set_value(receiptKeyCode, (char*)token.data.scalar.value, receipt);

                    if (ret != PPKG_OK) {
                        goto finalize;
                    }
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
        ret = uppm_receipt_check(receipt, receiptFilePath);
    }

    if (ret == PPKG_OK) {
        (*out) = receipt;
        return PPKG_OK;
    } else {
        uppm_receipt_free(receipt);
        return ret;
    }
}
