#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>
#include <time.h>
#include <yaml.h>
#include "ppkg.h"
#include "core/fs.h"
#include "core/regex/regex.h"

typedef enum {
    PPKGFormulaRepoKeyCode_unknown,
    PPKGFormulaRepoKeyCode_url,
    PPKGFormulaRepoKeyCode_branch,
    PPKGFormulaRepoKeyCode_pinned,
    PPKGFormulaRepoKeyCode_timestamp_added,
    PPKGFormulaRepoKeyCode_timestamp_last_updated
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
    printf("timestamp-added:         %s\n", formulaRepo->timestamp_added);
    printf("timestamp-last-updated:  %s\n", formulaRepo->timestamp_last_updated);

    if (formulaRepo->timestamp_added != NULL) {
        time_t tt = (time_t)atol(formulaRepo->timestamp_added);
        struct tm *tms = localtime(&tt);

        char buff[26] = {0};
        strftime(buff, 26, "%Y-%m-%d %H:%M:%S%z", tms);

        buff[24] = buff[23];
        buff[23] = buff[22];
        buff[22] = ':';

        printf("timestamp-added2:        %s\n", buff);
    }

    if (formulaRepo->timestamp_last_updated != NULL) {
        time_t tt = (time_t)atol(formulaRepo->timestamp_last_updated);
        struct tm *tms = localtime(&tt);

        char buff[26] = {0};
        strftime(buff, 26, "%Y-%m-%d %H:%M:%S%z", tms);

        buff[24] = buff[23];
        buff[23] = buff[22];
        buff[22] = ':';

        printf("timestamp-last-updated2: %s\n", buff);
    }

    printf("\n");
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

    if (formulaRepo->timestamp_added != NULL) {
        free(formulaRepo->timestamp_added);
        formulaRepo->timestamp_added = NULL;
    }

    if (formulaRepo->timestamp_last_updated != NULL) {
        free(formulaRepo->timestamp_last_updated);
        formulaRepo->timestamp_last_updated = NULL;
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
    } else if (strcmp(key, "timestamp-added") == 0) {
        return PPKGFormulaRepoKeyCode_timestamp_added;
    } else if (strcmp(key, "timestamp-last-updated") == 0) {
        return PPKGFormulaRepoKeyCode_timestamp_last_updated;
    } else {
        return PPKGFormulaRepoKeyCode_unknown;
    }
}

static void ppkg_formula_repo_set_value(PPKGFormulaRepoKeyCode keyCode, char * value, PPKGFormulaRepo * formulaRepo) {
    if (keyCode == PPKGFormulaRepoKeyCode_unknown) {
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
        case PPKGFormulaRepoKeyCode_url:    if (formulaRepo->url    != NULL) free(formulaRepo->url);    formulaRepo->url    = strdup(value); break;
        case PPKGFormulaRepoKeyCode_branch: if (formulaRepo->branch != NULL) free(formulaRepo->branch); formulaRepo->branch = strdup(value); break;
        case PPKGFormulaRepoKeyCode_timestamp_added:
            if (formulaRepo->timestamp_added != NULL) {
                free(formulaRepo->timestamp_added);
            }
            formulaRepo->timestamp_added = strdup(value);
            break;
        case PPKGFormulaRepoKeyCode_timestamp_last_updated:
            free(formulaRepo->timestamp_last_updated);
            formulaRepo->timestamp_last_updated = strdup(value);
            break;
        case PPKGFormulaRepoKeyCode_pinned:
            if (strcmp(value, "yes") == 0) {
                formulaRepo->pinned = true;
            }
            break;
        default:
            break;
    }
}

static int ppkg_formula_repo_check(PPKGFormulaRepo * formulaRepo, const char * formulaRepoConfigFilePath) {
    if (formulaRepo->url == NULL) {
        fprintf(stderr, "scheme error in formula repo config file: %s : summary mapping not found.\n", formulaRepoConfigFilePath);
        return PPKG_FORMULA_REPO_CONFIG_SCHEME_ERROR;
    }

    if (formulaRepo->branch == NULL) {
        fprintf(stderr, "scheme error in formula repo config file: %s : summary mapping not found.\n", formulaRepoConfigFilePath);
        return PPKG_FORMULA_REPO_CONFIG_SCHEME_ERROR;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formulaRepo->timestamp_added == NULL) {
        fprintf(stderr, "scheme error in formula repo config file: %s : timestamp-added mapping not found.\n", formulaRepoConfigFilePath);
        return PPKG_FORMULA_REPO_CONFIG_SCHEME_ERROR;
    }

    size_t i = 0;
    char   c;

    for (;; i++) {
        c = formulaRepo->timestamp_added[i];

        if (c == '\0') {
            break;
        }

        if ((c < '0') || (c > '9')) {
            fprintf(stderr, "scheme error in formula repo config file: %s : timestamp-added mapping's value should only contains non-numeric characters.\n", formulaRepoConfigFilePath);
            return PPKG_FORMULA_REPO_CONFIG_SCHEME_ERROR;
        }
    }

    if (i != 10) {
        fprintf(stderr, "scheme error in formula repo config file: %s : timestamp-added mapping's value's length must be 10.\n", formulaRepoConfigFilePath);
        return PPKG_FORMULA_REPO_CONFIG_SCHEME_ERROR;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formulaRepo->timestamp_last_updated != NULL) {
        i = 0;

        for (;; i++) {
            c = formulaRepo->timestamp_last_updated[i];

            if (c == '\0') {
                break;
            }

            if ((c < '0') || (c > '9')) {
                fprintf(stderr, "scheme error in formula repo config file: %s : timestamp-last-updated mapping's value should only contains non-numeric characters.\n", formulaRepoConfigFilePath);
                return PPKG_FORMULA_REPO_CONFIG_SCHEME_ERROR;
            }
        }

        if (i != 10) {
            fprintf(stderr, "scheme error in formula repo config file: %s : timestamp-last-updated mapping's value's length must be 10.\n", formulaRepoConfigFilePath);
            return PPKG_FORMULA_REPO_CONFIG_SCHEME_ERROR;
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

    int resultCode = PPKG_OK;

    do {
        // https://libyaml.docsforge.com/master/api/yaml_parser_scan/
        if (yaml_parser_scan(&parser, &token) == 0) {
            fprintf(stderr, "syntax error in formula repo config file: %s\n", formulaRepoConfigFilePath);
            resultCode = PPKG_FORMULA_REPO_CONFIG_SYNTAX_ERROR;
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
                    formulaRepoKeyCode = ppkg_formula_repo_key_code_from_key_name((char*)token.data.scalar.value);
                } else if (lastTokenType == 2) {
                    if (formulaRepo == NULL) {
                        formulaRepo = (PPKGFormulaRepo*)calloc(1, sizeof(PPKGFormulaRepo));

                        if (formulaRepo == NULL) {
                            resultCode = PPKG_ERROR_ALLOCATE_MEMORY_FAILED;
                            goto clean;
                        }
                    }
                    ppkg_formula_repo_set_value(formulaRepoKeyCode, (char*)token.data.scalar.value, formulaRepo);
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

    if (resultCode == PPKG_OK) {
        resultCode = ppkg_formula_repo_check(formulaRepo, formulaRepoConfigFilePath);

        if (resultCode == PPKG_OK) {
            //ppkg_formula_dump(formula);

            (*out) = formulaRepo;
            return PPKG_OK;
        }
    }

    //ppkg_formula_repo_dump(formulaRepo);

    ppkg_formula_repo_free(formulaRepo);

    return resultCode;
}
