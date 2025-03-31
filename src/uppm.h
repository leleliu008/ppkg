#ifndef UPPM_H
#define UPPM_H

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define UPPM_VERSION "20000.0.0"
#define UPPM_VERSION_MAJOR 20000
#define UPPM_VERSION_MINOR 0
#define UPPM_VERSION_PATCH 0

//////////////////////////////////////////////////////////////////////

typedef struct {
    char * name;
    char * url;
    char * branch;
    char * path;
    char * createdAt;
    char * updatedAt;
    int    pinned;
    int    enabled;
} UPPMFormulaRepo ;

int  uppm_formula_repo_sync_official_core();
int  uppm_formula_repo_parse(const char * formulaRepoConfigFilePath, UPPMFormulaRepo * * formulaRepo);
void uppm_formula_repo_free(UPPMFormulaRepo * formulaRepo);
void uppm_formula_repo_dump(UPPMFormulaRepo * formulaRepo);

typedef struct {
    char * summary;
    char * version;
    char * license;
    char * webpage;
    char * bin_url;
    char * bin_sha;
    char * dep_pkg;
    char * unpackd;
    char * install;
    char * path;
} UPPMFormula;

int  uppm_formula_lookup(const char * packageName, UPPMFormula * * formula);
void uppm_formula_free(UPPMFormula * formula);
void uppm_formula_dump(UPPMFormula * formula);

//////////////////////////////////////////////////////////////////////

typedef struct {
    char * summary;
    char * version;
    char * license;
    char * webpage;
    char * bin_url;
    char * bin_sha;
    char * dep_pkg;
    char * install;

    char * signature;
    char * timestamp;
} UPPMReceipt;

int  uppm_receipt_parse(const char * packageName, UPPMReceipt * * receipt);
void uppm_receipt_free(UPPMReceipt * receipt);
void uppm_receipt_dump(UPPMReceipt * receipt);

//////////////////////////////////////////////////////////////////////

/** get the uppm home directory absolute path
 *
 *  the capacity of buf must be PATH_MAX
 *
 *  len can be null if you do not want to known the length of filled string
 *
 *  on success, 0 is returned and buf will be filled with a null-terminated string
 *
 *  on error, none-zero value will be returned and buf remains unchanged.
 */
int uppm_home_dir(char buf[], size_t * len);

int uppm_install(const char * packageName, const bool verbose, const bool force);

int uppm_check_if_the_given_argument_matches_package_name_pattern(const char * arg);

int uppm_check_if_the_given_package_is_available(const char * packageName);
int uppm_check_if_the_given_package_is_installed(const char * packageName);
int uppm_check_if_the_given_package_is_outdated (const char * packageName);

#endif
