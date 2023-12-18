#ifndef PPKG_H
#define PPKG_H

#include <config.h>

#include <stdlib.h>
#include <stdbool.h>

#include "core/sysinfo.h"
#include "core/tar.h"


#define PPKG_PACKAGE_NAME_PATTERN "^[A-Za-z0-9+-._@]{1,50}$"

 
#define PPKG_OK                     0
#define PPKG_ERROR                  1

#define PPKG_ERROR_ARG_IS_NULL      2
#define PPKG_ERROR_ARG_IS_EMPTY     3
#define PPKG_ERROR_ARG_IS_INVALID   4
#define PPKG_ERROR_ARG_IS_UNKNOWN   5

#define PPKG_ERROR_MEMORY_ALLOCATE  6

#define PPKG_ERROR_SHA256_MISMATCH  7

#define PPKG_ERROR_ENV_HOME_NOT_SET 8
#define PPKG_ERROR_ENV_PATH_NOT_SET 9

#define PPKG_ERROR_NOT_FOUND    10
#define PPKG_ERROR_NOT_MATCH    11

#define PPKG_ERROR_PACKAGE_NOT_AVAILABLE 20
#define PPKG_ERROR_PACKAGE_NOT_INSTALLED 21
#define PPKG_ERROR_PACKAGE_NOT_OUTDATED  22
#define PPKG_ERROR_PACKAGE_IS_BROKEN     23

#define PPKG_ERROR_FORMULA_REPO_NOT_FOUND 30
#define PPKG_ERROR_FORMULA_REPO_HAS_EXIST 31
#define PPKG_ERROR_FORMULA_REPO_IS_BROKEN 32
#define PPKG_ERROR_FORMULA_REPO_CONFIG_SYNTAX 34
#define PPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME 35

#define PPKG_ERROR_FORMULA_SYNTAX     40
#define PPKG_ERROR_FORMULA_SCHEME     41

#define PPKG_ERROR_RECEIPT_SYNTAX     45
#define PPKG_ERROR_RECEIPT_SCHEME     46

#define PPKG_ERROR_URL_TRANSFORM_ENV_NOT_SET           50
#define PPKG_ERROR_URL_TRANSFORM_ENV_VALUE_IS_EMPTY    51
#define PPKG_ERROR_URL_TRANSFORM_ENV_POINT_TO_PATH_NOT_EXIST 52
#define PPKG_ERROR_URL_TRANSFORM_RUN_NO_RESULT         53

// libgit's error [-35, -1]
#define PPKG_ERROR_LIBGIT2_BASE    70

// libarchive's error [-30, 1]
#define PPKG_ERROR_ARCHIVE_BASE    110

// libcurl's error [1, 99]
#define PPKG_ERROR_NETWORK_BASE    150


typedef struct {
    char * summary;
    char * version;
    char * license;

    char * web_url;

    char * git_url;
    char * git_sha;
    char * git_ref;
    size_t git_nth;

    char * src_url;
    char * src_uri;
    char * src_sha;
    bool   src_is_dir;

    char * fix_url;
    char * fix_uri;
    char * fix_sha;

    char * res_url;
    char * res_uri;
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

    bool   sfslink;

    char * ppflags;
    char * ccflags;
    char * xxflags;
    char * ldflags;

    char * dopatch;
    char * install;

    char * path;

    bool web_url_is_calculated;
    bool version_is_calculated;
    bool bsystem_is_calculated;

    bool useBuildSystemAutogen;
    bool useBuildSystemAutotools;
    bool useBuildSystemConfigure;
    bool useBuildSystemCmake;
    bool useBuildSystemXmake;
    bool useBuildSystemGmake;
    bool useBuildSystemMeson;
    bool useBuildSystemNinja;
    bool useBuildSystemCargo;
    bool useBuildSystemGolang;

} PPKGFormula;

int  ppkg_formula_parse(const char * formulaFilePath, PPKGFormula * * out);
int  ppkg_formula_lookup(const char * packageName, const char * targetPlatformName, PPKGFormula * * formula);
int  ppkg_formula_locate(const char * packageName, const char * targetPlatformName, char * * out);
int  ppkg_formula_edit(const char * packageName, const char * targetPlatformName, const char * editor);
int  ppkg_formula_view(const char * packageName, const char * targetPlatformName, bool raw);
int  ppkg_formula_cat (const char * packageName, const char * targetPlatformName);
int  ppkg_formula_bat (const char * packageName, const char * targetPlatformName);


void ppkg_formula_free(PPKGFormula * formula);
void ppkg_formula_dump(PPKGFormula * formula);

//////////////////////////////////////////////////////////////////////

typedef struct {
    char * name;
    char * url;
    char * branch;
    char * path;
    char * createdAt;
    char * updatedAt;
    bool   pinned;
    bool   enabled;
} PPKGFormulaRepo ;

typedef struct {
    PPKGFormulaRepo * * repos;
    size_t size;
} PPKGFormulaRepoList ;

int  ppkg_formula_repo_create(const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled);
int  ppkg_formula_repo_add   (const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled);
int  ppkg_formula_repo_remove(const char * formulaRepoName);
int  ppkg_formula_repo_sync_ (const char * formulaRepoName);
int  ppkg_formula_repo_info_ (const char * formulaRepoName);
int  ppkg_formula_repo_config(const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled);
int  ppkg_formula_repo_config_write(const char * formulaRepoDIRPath, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled, const char * createdAt, const char * updatedAt);
int  ppkg_formula_repo_lookup(const char * formulaRepoName, PPKGFormulaRepo * * formulaRepo);
int  ppkg_formula_repo_parse (const char * formulaRepoConfigFilePath, PPKGFormulaRepo * * formulaRepo);

void ppkg_formula_repo_free(PPKGFormulaRepo * formulaRepo);
void ppkg_formula_repo_dump(PPKGFormulaRepo * formulaRepo);
int  ppkg_formula_repo_info(PPKGFormulaRepo * formulaRepo);
int  ppkg_formula_repo_sync(PPKGFormulaRepo * formulaRepo);

int  ppkg_formula_repo_list     (PPKGFormulaRepoList * * p);
void ppkg_formula_repo_list_free(PPKGFormulaRepoList   * p);

int  ppkg_formula_repo_list_printf();
int  ppkg_formula_repo_list_update();


//////////////////////////////////////////////////////////////////////

typedef struct {
    char * name;
    char * version;
    char * arch;
} PPKGTargetPlatform;

int ppkg_inspect_target_platform(const char * str, PPKGTargetPlatform * targetPlatform);

int ppkg_inspect_package_spec(const char * str, char ** packageName, PPKGTargetPlatform * targetPlatform);

//////////////////////////////////////////////////////////////////////

typedef struct {
    char * summary;
    char * version;
    char * license;

    char * web_url;

    char * git_url;
    char * git_sha;
    char * git_ref;
    size_t git_nth;

    char * src_url;
    char * src_uri;
    char * src_sha;

    char * fix_url;
    char * fix_uri;
    char * fix_sha;

    char * res_url;
    char * res_uri;
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

    char * ppflags;
    char * ccflags;
    char * xxflags;
    char * ldflags;

    char * dopatch;
    char * install;

    char * path;

    char * builtBy;
    char * builtAt;
    char * builtFor;
} PPKGReceipt;

int  ppkg_receipt_parse(const char * packageName, const PPKGTargetPlatform * targetPlatform, PPKGReceipt * * receipt);
void ppkg_receipt_free(PPKGReceipt * receipt);
void ppkg_receipt_dump(PPKGReceipt * receipt);

//////////////////////////////////////////////////////////////////////

typedef struct {
    char * cc;
    char * objc;
    char * cxx;
    char * cpp;
    char * as;
    char * ar;
    char * ranlib;
    char * ld;
    char * nm;
    char * strip;
    char * size;
    char * strings;
    char * objdump;
    char * objcopy;
    char * readelf;
    char * dlltool;
    char * addr2line;

    char * sysroot;

    char * ccflags;

    char * cxxflags;

    char * cppflags;

    char * ldflags;
} PPKGToolChain;

int  ppkg_toolchain_locate(PPKGToolChain * toolchain);
void ppkg_toolchain_free(PPKGToolChain toolchain);
void ppkg_toolchain_dump(PPKGToolChain toolchain);

//////////////////////////////////////////////////////////////////////

int ppkg_main(int argc, char* argv[]);

int ppkg_util(int argc, char* argv[]);

int ppkg_help();

int ppkg_sysinfo();

int ppkg_buildinfo();

int ppkg_env(bool verbose);

int ppkg_home_dir(char buf[], size_t bufSize, size_t * outSize);

int ppkg_session_dir(char buf[], size_t bufSize, size_t * outSize);

int ppkg_search(const char * regPattern, const char * targetPlatformName);

int ppkg_available_info(const char * packageName, const char * targetPlatformName, const char * key);

int ppkg_installed_info(const char * packageName, const PPKGTargetPlatform * targetPlatform, const char * key);

int ppkg_tree(const char * packageName, const PPKGTargetPlatform * targetPlatform, size_t argc, char* argv[]);

int ppkg_logs(const char * packageName, const PPKGTargetPlatform * targetPlatform);

int ppkg_pack(const char * packageName, const PPKGTargetPlatform * targetPlatform, ArchiveType outputType, const char * outputPath, bool verbose);

typedef enum {
    PPKGDependsOutputType_DOT,
    PPKGDependsOutputType_BOX,
    PPKGDependsOutputType_SVG,
    PPKGDependsOutputType_PNG,
} PPKGDependsOutputType;

int ppkg_depends(const char * packageName, const char * targetPlatformName, PPKGDependsOutputType outputType, const char * outputPath);

int ppkg_fetch(const char * packageName, const char * targetPlatformName, bool verbose);

//////////////////////////////////////////////////////////////////////

typedef enum {
    PPKGLogLevel_silent,
    PPKGLogLevel_normal,
    PPKGLogLevel_verbose,
    PPKGLogLevel_very_verbose
} PPKGLogLevel;

typedef enum {
    PPKGBuildType_release,
    PPKGBuildType_debug
} PPKGBuildType;

typedef enum {
    PPKGLinkType_shared_most,
    PPKGLinkType_shared_full,
    PPKGLinkType_static_most,
    PPKGLinkType_static_full
} PPKGLinkType;

typedef struct {
    bool   exportCompileCommandsJson;
    bool   keepSessionDIR;
    bool   enableCcache;
    bool   enableBear;
    bool   dryrun;
    bool   force;

    bool   xtrace;

    bool   verbose_net;
    bool   verbose_env;
    bool   verbose_cc;
    bool   verbose_ld;

    size_t parallelJobsCount;

    PPKGBuildType buildType;
    PPKGLinkType  linkType;

    PPKGLogLevel  logLevel;
} PPKGInstallOptions;

int ppkg_install  (const char * packageName, const PPKGTargetPlatform * targetPlatform, const PPKGInstallOptions * options);

int ppkg_upgrade  (const char * packageName, const PPKGTargetPlatform * targetPlatform, const PPKGInstallOptions * options);

int ppkg_reinstall(const char * packageName, const PPKGTargetPlatform * targetPlatform, const PPKGInstallOptions * options);

int ppkg_uninstall(const char * packageName, const PPKGTargetPlatform * targetPlatform, bool verbose);

int ppkg_upgrade_self(bool verbose);

int ppkg_integrate_zsh_completion (const char * outputDIR, bool verbose);
int ppkg_integrate_bash_completion(const char * outputDIR, bool verbose);
int ppkg_integrate_fish_completion(const char * outputDIR, bool verbose);

int ppkg_setup(bool verbose);

int ppkg_cleanup(bool verbose);

int ppkg_check_if_the_given_argument_matches_package_name_pattern(const char * arg);

int ppkg_check_if_the_given_package_is_available(const char * packageName, const char * targetPlatformName);
int ppkg_check_if_the_given_package_is_installed(const char * packageName, const PPKGTargetPlatform * targetPlatform);
int ppkg_check_if_the_given_package_is_outdated (const char * packageName, const PPKGTargetPlatform * targetPlatform);

typedef int (*PPKGPackageNameCallbak)(const char * packageName, const char * targetPlatformName, size_t index, const void * payload);

int ppkg_list_the_available_packages(const char * targetPlatformName, PPKGPackageNameCallbak packageNameCallbak, const void * payload);
int ppkg_list_the_installed_packages(const PPKGTargetPlatform * targetPlatform);
int ppkg_list_the_outdated__packages(const PPKGTargetPlatform * targetPlatform);

int ppkg_show_the_available_packages(const char * targetPlatformName);

int ppkg_git_sync(const char * gitRepositoryDIRPath, const char * remoteUrl, const char * remoteRef, const char * remoteTrackingRef, const char * checkoutToBranchName, const size_t fetchDepth);

int ppkg_generate_url_transform_sample();

int ppkg_examine_filetype_from_url(const char * url, char buf[], size_t bufSize);

int ppkg_examine_filename_from_url(const char * url, char buf[], size_t bufSize);

int ppkg_http_fetch_to_file(const char * url, const char * outputFilePath, bool verbose, bool showProgress);

int ppkg_http_fetch_to_stream(const char * url, FILE * stream, bool verbose, bool showProgress);

int ppkg_download(const char * url, const char * uri, const char * expectedSHA256SUM, const char * outputPath, bool verbose);

int ppkg_uncompress(const char * filePath, const char * unpackDIR, size_t stripComponentsNumber, bool verbose);

int ppkg_rename_or_copy_file(const char * fromFilePath, const char * toFilePath);

int ppkg_copy_file(const char * fromFilePath, const char * toFilePath);

int ppkg_mkdir_p(const char * dirPath, bool verbose);

int ppkg_rm_r(const char * dirPath, bool verbose);

int ppkg_setenv_SSL_CERT_FILE();

#endif
