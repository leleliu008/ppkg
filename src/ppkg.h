#ifndef PPKG_H
#define PPKG_H

#include <config.h>
#include <stdlib.h>
#include <stdbool.h>
#include "core/untar.h"

#ifndef PPKG_VERSION
#define PPKG_VERSION "0.0.0"
#endif

#define PPKG_PACKAGE_NAME_PATTERN "^[A-Za-z0-9+-._@]{1,50}$"

#define PPKG_OK               0
#define PPKG_ERROR            1

#define PPKG_ARG_IS_NULL      10
#define PPKG_ARG_IS_EMPTY     11
#define PPKG_ARG_IS_INVALID   12
#define PPKG_ARG_IS_UNKNOWN   13

#define PPKG_ENV_HOME_NOT_SET 5
#define PPKG_ENV_PATH_NOT_SET 6

#define PPKG_NETWORK_ERROR    7

#define PPKG_SHA256_MISMATCH  8

#define PPKG_PACKAGE_IS_NOT_AVAILABLE 25
#define PPKG_PACKAGE_IS_NOT_INSTALLED 26
#define PPKG_PACKAGE_IS_NOT_OUTDATED  27

#define PPKG_FORMULA_REPO_NOT_EXIST   30

#define PPKG_FORMULA_SYNTAX_ERROR     40
#define PPKG_FORMULA_SCHEME_ERROR     41
  
#define PPKG_RECEIPT_SYNTAX_ERROR     50
#define PPKG_RECEIPT_SCHEME_ERROR     51


void ppkg_show_error_message(int errorCode, const char * str);

typedef struct {
    char * summary;
    char * version;
    char * license;

    char * web_url;

    char * git_url;
    char * git_sha;
    char * git_ref;
    bool   shallow;

    char * src_url;
    char * src_sha;

    char * fix_url;
    char * fix_sha;

    char * res_url;
    char * res_sha;

    char * dep_pkg;
    char * dep_upp;
    char * dep_pym;
    char * dep_plm;

    char * bsystem;
    char * bscript;

    bool   binbstd;
    bool   parallel;

    bool   symlink;

    char * cdefine;
    char * ccflags;
    char * xxflags;
    char * ldflags;

    char * exetype;

    char * prepare;
    char * install;

    char * path;
} PPKGFormula;

int  ppkg_formula_parse (const char * packageName, PPKGFormula * * formula);
int  ppkg_formula_path  (const char * packageName, char * * out);
int  ppkg_formula_cat   (const char * packageName);
int  ppkg_formula_bat   (const char * packageName);

void ppkg_formula_free(PPKGFormula * formula);
void ppkg_formula_dump(PPKGFormula * formula);

//////////////////////////////////////////////////////////////////////

typedef struct {
    char * name;
    char * url;
    char * branch;
    char * path;
} PPKGFormulaRepo ;

typedef struct {
    PPKGFormulaRepo * * repos;
    size_t size;
} PPKGFormulaRepoList ;

int  ppkg_formula_repo_list_new (PPKGFormulaRepoList * * p);
void ppkg_formula_repo_list_free(PPKGFormulaRepoList   * p);

int  ppkg_formula_repo_list_printf();
int  ppkg_formula_repo_list_update();

int   ppkg_formula_repo_add(const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName);
int   ppkg_formula_repo_del(const char * formulaRepoName);

//////////////////////////////////////////////////////////////////////

typedef struct {
    char * summary;
    char * version;
    char * license;

    char * web_url;

    char * git_url;
    char * git_sha;
    char * git_ref;
    bool   shallow;

    char * src_url;
    char * src_sha;

    char * fix_url;
    char * fix_sha;

    char * res_url;
    char * res_sha;

    char * dep_pkg;
    char * dep_upp;
    char * dep_pym;
    char * dep_plm;

    char * bsystem;
    char * bscript;

    bool   binbstd;
    bool   parallel;

    bool   symlink;

    char * cdefine;
    char * ccflags;
    char * xxflags;
    char * ldflags;

    char * exetype;

    char * prepare;
    char * install;

    char * path;

    char * signature;
    char * timestamp;
} PPKGReceipt;

int  ppkg_receipt_parse(const char * packageName, PPKGReceipt * * receipt);
void ppkg_receipt_free(PPKGReceipt * receipt);
void ppkg_receipt_dump(PPKGReceipt * receipt);

//////////////////////////////////////////////////////////////////////

typedef enum {
    PPKGDependsOutputFormat_DOT,
    PPKGDependsOutputFormat_BOX,
    PPKGDependsOutputFormat_SVG,
    PPKGDependsOutputFormat_PNG,
} PPKGDependsOutputFormat;

//////////////////////////////////////////////////////////////////////

int ppkg_main(int argc, char* argv[]);

int ppkg_help();

int ppkg_env(bool verbose);

int ppkg_search(const char * keyword);

int ppkg_info(const char * packageName, const char * key);

int ppkg_tree(const char * packageName, size_t argc, char* argv[]);

int ppkg_logs(const char * packageName);

int ppkg_pack(const char * packageName, ArchiveType type, bool verbose);

int ppkg_depends(const char * packageName, PPKGDependsOutputFormat outputFormat);

int ppkg_fetch(const char * packageName, bool verbose);

int ppkg_install(const char * packageName, bool verbose);

int ppkg_reinstall(const char * packageName, bool verbose);

int ppkg_uninstall(const char * packageName, bool verbose);

int ppkg_upgrade(const char * packageName, bool verbose);

int ppkg_upgrade_self(bool verbose);

int ppkg_integrate_zsh_completion (const char * outputDir, bool verbose);
int ppkg_integrate_bash_completion(const char * outputDir, bool verbose);
int ppkg_integrate_fish_completion(const char * outputDir, bool verbose);

int ppkg_cleanup(bool verbose);

int ppkg_check_if_the_given_argument_matches_package_name_pattern(const char * arg);

int ppkg_check_if_the_given_package_is_available(const char * packageName);
int ppkg_check_if_the_given_package_is_installed(const char * packageName);
int ppkg_check_if_the_given_package_is_outdated (const char * packageName);

int ppkg_list_the_available_packages();
int ppkg_list_the_installed_packages();
int ppkg_list_the_outdated__packages();


int ppkg_util_base64_encode_of_string(const char * str);
int ppkg_util_base64_decode_to_string(const char * base64EncodedStr);

#endif
