#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <yaml.h>

#include "ppkg.h"

typedef enum {
    PPKGFormulaRepoKeyCode_unknown,
    PPKGFormulaRepoKeyCode_url,
    PPKGFormulaRepoKeyCode_branch,
    PPKGFormulaRepoKeyCode_pinned,
    PPKGFormulaRepoKeyCode_enabled,
    PPKGFormulaRepoKeyCode_timestamp_created,
    PPKGFormulaRepoKeyCode_timestamp_updated
} PPKGFormulaRepoKeyCode;

void ppkg_formula_repo_dump(PPKGFormulaRepo * formulaRepo) {
    if (formulaRepo == NULL) {
        return;
    }

    printf("name: %s\n", formulaRepo->name);
    printf("path: %s\n", formulaRepo->path);
    printf("url:  %s\n", formulaRepo->url);
    printf("branch: %s\n", formulaRepo->branch);
    printf("pinned: %s\n", formulaRepo->pinned ? "yes" : "no");
    printf("enabled: %s\n", formulaRepo->enabled ? "yes" : "no");
    printf("timestamp-created: %s\n", formulaRepo->timestamp_created);
    printf("timestamp-updated: %s\n", formulaRepo->timestamp_updated);
}

void ppkg_formula_repo_free(PPKGFormulaRepo * formulaRepo) {
    if (formulaRepo == NULL) {
        return;
    }

    if (formulaRepo->name != NULL) {
        free(formulaRepo->name);
        formulaRepo->name = NULL;
    }

    if (formulaRepo->path != NULL) {
        free(formulaRepo->path);
        formulaRepo->path = NULL;
    }

    if (formulaRepo->url != NULL) {
        free(formulaRepo->url);
        formulaRepo->url = NULL;
    }

    if (formulaRepo->branch != NULL) {
        free(formulaRepo->branch);
        formulaRepo->branch = NULL;
    }

    if (formulaRepo->timestamp_created != NULL) {
        free(formulaRepo->timestamp_created);
        formulaRepo->timestamp_created = NULL;
    }

    if (formulaRepo->timestamp_updated != NULL) {
        free(formulaRepo->timestamp_updated);
        formulaRepo->timestamp_updated = NULL;
    }

    free(formulaRepo);
}

static PPKGFormulaRepoKeyCode ppkg_formula_repo_key_code_from_key_name(char * key) {
           if (strcmp(key, "url") == 0) {
        return PPKGFormulaRepoKeyCode_url;
    } else if (strcmp(key, "branch") == 0) {
        return PPKGFormulaRepoKeyCode_branch;
    } else if (strcmp(key, "pinned") == 0) {
        return PPKGFormulaRepoKeyCode_pinned;
    } else if (strcmp(key, "enabled") == 0) {
        return PPKGFormulaRepoKeyCode_enabled;
    } else if (strcmp(key, "timestamp-created") == 0) {
        return PPKGFormulaRepoKeyCode_timestamp_created;
    } else if (strcmp(key, "timestamp-updated") == 0) {
        return PPKGFormulaRepoKeyCode_timestamp_updated;
    } else {
        return PPKGFormulaRepoKeyCode_unknown;
    }
}

static int ppkg_formula_repo_set_value(PPKGFormulaRepoKeyCode keyCode, char * value, PPKGFormulaRepo * formulaRepo) {
    if (keyCode == PPKGFormulaRepoKeyCode_unknown) {
        return PPKG_OK;
    }

    char c;

    for (;;) {
        c = value[0];

        if (c == '\0') {
            return PPKG_OK;
        }

        // non-printable ASCII characters and space
        if (c <= 32) {
            value = &value[1];
        } else {
            break;
        }
    }

    switch (keyCode) {
        case PPKGFormulaRepoKeyCode_url:
            if (formulaRepo->url != NULL) {
                free(formulaRepo->url);
            }

            formulaRepo->url = strdup(value);

            return formulaRepo->url == NULL ? PPKG_ERROR_MEMORY_ALLOCATE : PPKG_OK;
        case PPKGFormulaRepoKeyCode_branch:
            if (formulaRepo->branch != NULL) {
                free(formulaRepo->branch);
            }

            formulaRepo->branch = strdup(value);

            return formulaRepo->branch == NULL ? PPKG_ERROR_MEMORY_ALLOCATE : PPKG_OK;
        case PPKGFormulaRepoKeyCode_timestamp_created:
            if (formulaRepo->timestamp_created != NULL) {
                free(formulaRepo->timestamp_created);
            }

            formulaRepo->timestamp_created = strdup(value);

            return formulaRepo->timestamp_created == NULL ? PPKG_ERROR_MEMORY_ALLOCATE : PPKG_OK;
        case PPKGFormulaRepoKeyCode_timestamp_updated:
            free(formulaRepo->timestamp_updated);
            formulaRepo->timestamp_updated = strdup(value);

            return formulaRepo->timestamp_updated == NULL ? PPKG_ERROR_MEMORY_ALLOCATE : PPKG_OK;
        case PPKGFormulaRepoKeyCode_pinned:
            if (strcmp(value, "1") == 0) {
                formulaRepo->pinned = 1;
                return PPKG_OK;
            } else if (strcmp(value, "0") == 0) {
                formulaRepo->pinned = 0;
                return PPKG_OK;
            } else {
                return PPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
            }
        case PPKGFormulaRepoKeyCode_enabled:
            if (strcmp(value, "1") == 0) {
                formulaRepo->enabled = 1;
                return PPKG_OK;
            } else if (strcmp(value, "0") == 0) {
                formulaRepo->enabled = 0;
                return PPKG_OK;
            } else {
                return PPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
            }
        case PPKGFormulaRepoKeyCode_unknown:
        default:
            return PPKG_OK;
    }
}

static int ppkg_formula_repo_check(PPKGFormulaRepo * formulaRepo, const char * formulaRepoConfigFilePath) {
    if (formulaRepo->url == NULL) {
        fprintf(stderr, "scheme error in formula repo config file: %s : summary mapping not found.\n", formulaRepoConfigFilePath);
        return PPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
    }

    if (formulaRepo->branch == NULL) {
        fprintf(stderr, "scheme error in formula repo config file: %s : summary mapping not found.\n", formulaRepoConfigFilePath);
        return PPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formulaRepo->timestamp_created == NULL) {
        fprintf(stderr, "scheme error in formula repo config file: %s : timestamp-created mapping not found.\n", formulaRepoConfigFilePath);
        return PPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
    }

    size_t i = 0;
    char   c;

    for (;; i++) {
        c = formulaRepo->timestamp_created[i];

        if (c == '\0') {
            break;
        }

        if ((c < '0') || (c > '9')) {
            fprintf(stderr, "scheme error in formula repo config file: %s : timestamp-created mapping's value should only contains non-numeric characters.\n", formulaRepoConfigFilePath);
            return PPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
        }
    }

    if (i != 10) {
        fprintf(stderr, "scheme error in formula repo config file: %s : timestamp-created mapping's value's length must be 10.\n", formulaRepoConfigFilePath);
        return PPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formulaRepo->timestamp_updated != NULL) {
        i = 0;

        for (;; i++) {
            c = formulaRepo->timestamp_updated[i];

            if (c == '\0') {
                break;
            }

            if ((c < '0') || (c > '9')) {
                fprintf(stderr, "scheme error in formula repo config file: %s : timestamp-updated mapping's value should only contains non-numeric characters.\n", formulaRepoConfigFilePath);
                return PPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
            }
        }

        if (i != 10) {
            fprintf(stderr, "scheme error in formula repo config file: %s : timestamp-updated mapping's value's length must be 10.\n", formulaRepoConfigFilePath);
            return PPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
        }
    }

    return PPKG_OK;
}

int ppkg_formula_repo_parse(const char * formulaRepoConfigFilePath, PPKGFormulaRepo * * out) {
    FILE * file = fopen(formulaRepoConfigFilePath, "r");

    if (file == NULL) {
        perror(formulaRepoConfigFilePath);
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

    PPKGFormulaRepoKeyCode formulaRepoKeyCode = PPKGFormulaRepoKeyCode_unknown;

    PPKGFormulaRepo * formulaRepo = NULL;

    int lastTokenType = 0;

    int ret = PPKG_OK;

    do {
        // https://libyaml.docsforge.com/master/api/yaml_parser_scan/
        if (yaml_parser_scan(&parser, &token) == 0) {
            fprintf(stderr, "syntax error in formula repo config file: %s\n", formulaRepoConfigFilePath);
            ret = PPKG_ERROR_FORMULA_REPO_CONFIG_SYNTAX;
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
                    formulaRepoKeyCode = ppkg_formula_repo_key_code_from_key_name((char*)token.data.scalar.value);
                } else if (lastTokenType == 2) {
                    if (formulaRepo == NULL) {
                        formulaRepo = (PPKGFormulaRepo*)calloc(1, sizeof(PPKGFormulaRepo));

                        if (formulaRepo == NULL) {
                            ret = PPKG_ERROR_MEMORY_ALLOCATE;
                            goto finalize;
                        }

                        formulaRepo->enabled = 1;
                    }

                    ret = ppkg_formula_repo_set_value(formulaRepoKeyCode, (char*)token.data.scalar.value, formulaRepo);

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
        ret = ppkg_formula_repo_check(formulaRepo, formulaRepoConfigFilePath);
    }

    //ppkg_formula_repo_dump(formulaRepo);

    if (ret == PPKG_OK) {
        (*out) = formulaRepo;
        return PPKG_OK;
    } else {
        ppkg_formula_repo_free(formulaRepo);
        return ret;
    }
}
