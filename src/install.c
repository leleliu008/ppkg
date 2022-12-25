#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <libgen.h>
#include <sys/stat.h>

#include "core/fs.h"
#include "core/http.h"
#include "core/sysinfo.h"
#include "core/sha256sum.h"
#include "core/untar.h"
#include "core/util.h"
#include "core/rm-r.h"
#include "ppkg.h"

extern int record_installed_files(const char * installedDirPath);

static int ppkg_xx(char * packageName, char * * * depPackageNameArrayListP,  PPKGFormula * * * formulasP, size_t * sizeP, size_t * capcityP) {
    PPKGFormula * formula = NULL;

    int resultCode = ppkg_formula_parse(packageName, &formula);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    if ((*sizeP) == (*capcityP)) {
        size_t capcity = (*capcityP) + 5;

        char ** depPackageNameArrayList = (char**)realloc(*depPackageNameArrayListP, capcity * sizeof(char*));

        if (depPackageNameArrayList == NULL) {
            return -1;
        }

        PPKGFormula ** formulaArrayList = (PPKGFormula**)realloc(*formulasP, capcity * sizeof(PPKGFormula*));

        if (formulaArrayList == NULL) {
            return -1;
        }

        (*capcityP) = capcity;
        (*depPackageNameArrayListP) = depPackageNameArrayList;
        (*formulasP) = formulaArrayList;
    }

    (*depPackageNameArrayListP)[*sizeP] = packageName;
    (*formulasP)[*sizeP] = formula;

    (*sizeP)++;

    //////////////////////////////////////////////////////////////////////////////

    if (formula->dep_pkg != NULL) {
        size_t depPackageNamesLength = strlen(formula->dep_pkg);
        size_t depPackageNamesCopyLength = depPackageNamesLength + 1;
        char   depPackageNamesCopy[depPackageNamesCopyLength];
        memset(depPackageNamesCopy, 0, depPackageNamesCopyLength);
        strcpy(depPackageNamesCopy, formula->dep_pkg);

        char * p;
        char * depPackageName = strtok_r(depPackageNamesCopy, " ", &p);

        while (depPackageName != NULL) {
            resultCode = ppkg_xx(depPackageName, depPackageNameArrayListP, formulasP, sizeP, capcityP);

            if (resultCode != PPKG_OK) {
                return resultCode;
            }

            depPackageName = strtok_r(NULL, " ", &p);
        }
    }

    return PPKG_OK;
}

int ppkg_install(const char * packageName, PPKGInstallOptions options) {
    PPKGFormula * formula = NULL;

    int resultCode = ppkg_formula_parse(packageName, &formula);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->dep_pkg != NULL) {
        size_t depPackageNamesLength = strlen(formula->dep_pkg);
        size_t depPackageNamesCopyLength = depPackageNamesLength + 1;
        char   depPackageNamesCopy[depPackageNamesCopyLength];
        memset(depPackageNamesCopy, 0, depPackageNamesCopyLength);
        strcpy(depPackageNamesCopy, formula->dep_pkg);

        char * context;

        char * depPackageName = strtok_r(depPackageNamesCopy, " ", &context);

        while (depPackageName != NULL) {
            resultCode = ppkg_install(depPackageName, options);

            if (resultCode != PPKG_OK) {
                ppkg_formula_free(formula);
                return resultCode;
            }

            depPackageName = strtok_r(NULL, " ", &context);
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageNameLength = strlen(packageName);

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t  ppkgHomeDirLength = userHomeDirLength + 7;
    char    ppkgHomeDir[ppkgHomeDirLength];
    memset (ppkgHomeDir, 0, ppkgHomeDirLength);
    sprintf(ppkgHomeDir, "%s/.ppkg", userHomeDir);

    //////////////////////////////////////////////////////////////////////////////

    size_t  ppkgInstallingDirLength = ppkgHomeDirLength + packageNameLength + 12;
    char    ppkgInstallingDir[ppkgInstallingDirLength];
    memset (ppkgInstallingDir, 0, ppkgInstallingDirLength);
    sprintf(ppkgInstallingDir, "%s/installing", ppkgHomeDir);

    if (!exists_and_is_a_directory(ppkgInstallingDir)) {
        if (mkdir(ppkgInstallingDir, S_IRWXU) != 0) {
            perror(ppkgInstallingDir);
            ppkg_formula_free(formula);
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t  packageInstallingDirLength = ppkgInstallingDirLength + packageNameLength + 2;
    char    packageInstallingDir[packageInstallingDirLength];
    memset (packageInstallingDir, 0, packageInstallingDirLength);
    sprintf(packageInstallingDir, "%s/%s", ppkgInstallingDir, packageName);

    if (exists_and_is_a_directory(packageInstallingDir)) {
        if (rm_r(packageInstallingDir, options.verbose) != 0) {
            perror(packageInstallingDir);
            ppkg_formula_free(formula);
            return PPKG_ERROR;
        }
    }

    if (mkdir(packageInstallingDir, S_IRWXU) != 0) {
        perror(packageInstallingDir);
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t  packageInstalledDirLength = userHomeDirLength + packageNameLength + 20;
    char    packageInstalledDir[packageInstalledDirLength];
    memset (packageInstalledDir, 0, packageInstalledDirLength);
    sprintf(packageInstalledDir, "%s/.ppkg/installed/%s", userHomeDir, packageName);

    size_t  packageInstalledMetaInfoDirLength = packageInstalledDirLength + 6;
    char    packageInstalledMetaInfoDir[packageInstalledMetaInfoDirLength];
    memset (packageInstalledMetaInfoDir, 0, packageInstalledMetaInfoDirLength);
    sprintf(packageInstalledMetaInfoDir, "%s/.ppkg", packageInstalledDir);

    size_t  receiptFilePathLength = packageInstalledMetaInfoDirLength + 12;
    char    receiptFilePath[receiptFilePathLength];
    memset (receiptFilePath, 0, receiptFilePathLength);
    sprintf(receiptFilePath, "%s/receipt.yml", packageInstalledMetaInfoDir);

    if (exists_and_is_a_regular_file(receiptFilePath)) {
        fprintf(stderr, "package [%s] already has been installed.\n", packageName);
        return PPKG_OK;
    }

    //////////////////////////////////////////////////////////////////////////////

    fprintf(stderr, "prepare to install package [%s].\n", packageName);

    resultCode = ppkg_fetch(packageName, options.verbose);

    if (resultCode != PPKG_OK) {
        ppkg_formula_free(formula);
        return resultCode;
    }

    //////////////////////////////////////////////////////////////////////////////

    SysInfo * sysinfo = NULL;

    if (sysinfo_make(&sysinfo) != 0) {
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    char * libcName = NULL;

    switch(sysinfo->libc) {
        case LIBC_GLIBC: libcName = (char*)"glibc"; break;
        case LIBC_MUSL:  libcName = (char*)"musl";  break;
        default:         libcName = (char*)"";
    }

    //////////////////////////////////////////////////////////////////////////////

    char * currentExecutablePath = NULL;

    if (get_current_executable_realpath(&currentExecutablePath) < 0) {
        sysinfo_free(sysinfo);
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t  installShellScriptFilePathLength = packageInstallingDirLength + 12;
    char    installShellScriptFilePath[installShellScriptFilePathLength];
    memset (installShellScriptFilePath, 0, installShellScriptFilePathLength);
    sprintf(installShellScriptFilePath, "%s/install.sh", packageInstallingDir);

    FILE *  installShellScriptFile = fopen(installShellScriptFilePath, "w");

    if (installShellScriptFile == NULL) {
        perror(installShellScriptFilePath);
        sysinfo_free(sysinfo);
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    fprintf(installShellScriptFile, "set -ex\n\n");

    fprintf(installShellScriptFile, "TIMESTAMP_UNIX='%lu'\n\n", time(NULL));

    fprintf(installShellScriptFile, "NATIVE_OS_NCPU='%lu'\n", sysinfo->ncpu);
    fprintf(installShellScriptFile, "NATIVE_OS_ARCH='%s'\n", sysinfo->arch);
    fprintf(installShellScriptFile, "NATIVE_OS_KIND='%s'\n", sysinfo->kind);
    fprintf(installShellScriptFile, "NATIVE_OS_TYPE='%s'\n", sysinfo->type);
    fprintf(installShellScriptFile, "NATIVE_OS_NAME='%s'\n", sysinfo->name);
    fprintf(installShellScriptFile, "NATIVE_OS_VERS='%s'\n", sysinfo->vers);
    fprintf(installShellScriptFile, "NATIVE_OS_LIBC='%s'\n\n", libcName);

    fprintf(installShellScriptFile, "KEEP_INSTALLING_DIR='%s'\n", options.keepInstallingDir ? "yes" : "no");
    fprintf(installShellScriptFile, "DRYRUN='%s'\n", options.dryrun ? "yes" : "no");
    fprintf(installShellScriptFile, "BEAR_ENABLED='%s'\n", options.enableBear ? "yes" : "no");
    fprintf(installShellScriptFile, "CCACHE_ENABLED='%s'\n", options.enableCcache ? "yes" : "no");
    fprintf(installShellScriptFile, "EXPORT_COMPILE_COMMANDS_JSON='%s'\n", options.exportCompileCommandsJson ? "yes" : "no");
    fprintf(installShellScriptFile, "VERBOSE_LEVEL='%d'\n", options.verbose ? 1 : 1);
    fprintf(installShellScriptFile, "BUILD_TYPE='%s'\n", options.buildType == BuildType_release ? "release" : "debug");

    size_t njobs;

    if (formula->parallel) {
        if (options.parallelJobsCount > 0) {
            njobs = options.parallelJobsCount;
        } else {
            njobs = sysinfo->ncpu;
        }
    } else {
        njobs = 1;
    }

    fprintf(installShellScriptFile, "BUILD_NJOBS='%lu'\n", njobs);

    switch (options.LinkType) {
        case LinkType_static_only:     fprintf(installShellScriptFile, "LINK_TYPE='%s'\n", "static-only");
        case LinkType_shared_only:     fprintf(installShellScriptFile, "LINK_TYPE='%s'\n", "shared-only");
        case LinkType_static_prefered: fprintf(installShellScriptFile, "LINK_TYPE='%s'\n", "static-prefered");
        case LinkType_shared_prefered: fprintf(installShellScriptFile, "LINK_TYPE='%s'\n", "static-prefered");
    }

    fprintf(installShellScriptFile, "PPKG_VERSION='%s'\n", PPKG_VERSION);
    fprintf(installShellScriptFile, "PPKG_HOME='%s'\n", ppkgHomeDir);
    fprintf(installShellScriptFile, "PPKG_EXECUTABLE_PATH='%s'\n", currentExecutablePath);

    fprintf(installShellScriptFile, "PACKAGE_NAME='%s'\n", packageName);
    fprintf(installShellScriptFile, "PACKAGE_SUMMARY='%s'\n", formula->summary);
    fprintf(installShellScriptFile, "PACKAGE_VERSION='%s'\n", formula->version);
    fprintf(installShellScriptFile, "PACKAGE_LICENSE='%s'\n", formula->license == NULL ? "" : formula->license);
    fprintf(installShellScriptFile, "PACKAGE_WEB_URL='%s'\n", formula->web_url);

    fprintf(installShellScriptFile, "PACKAGE_GIT_URL='%s'\n", formula->git_url == NULL ? "" : formula->git_url);
    fprintf(installShellScriptFile, "PACKAGE_GIT_SHA='%s'\n", formula->git_sha == NULL ? "" : formula->git_sha);
    fprintf(installShellScriptFile, "PACKAGE_GIT_REF='%s'\n", formula->git_ref == NULL ? "" : formula->git_ref);

    fprintf(installShellScriptFile, "PACKAGE_SHALLOW='%s'\n", formula->shallow ? "yes" : "no");

    fprintf(installShellScriptFile, "PACKAGE_SRC_URL='%s'\n", formula->src_url == NULL ? "" : formula->src_url);
    fprintf(installShellScriptFile, "PACKAGE_SRC_SHA='%s'\n", formula->src_sha == NULL ? "" : formula->src_sha);

    fprintf(installShellScriptFile, "PACKAGE_FIX_URL='%s'\n", formula->fix_url == NULL ? "" : formula->fix_url);
    fprintf(installShellScriptFile, "PACKAGE_FIX_SHA='%s'\n", formula->fix_sha == NULL ? "" : formula->fix_sha);

    fprintf(installShellScriptFile, "PACKAGE_RES_URL='%s'\n", formula->res_url == NULL ? "" : formula->res_url);
    fprintf(installShellScriptFile, "PACKAGE_RES_SHA='%s'\n", formula->res_sha == NULL ? "" : formula->res_sha);

    fprintf(installShellScriptFile, "PACKAGE_DEP_PKG_R='%s'\n", "");
    fprintf(installShellScriptFile, "PACKAGE_DEP_PKG='%s'\n", formula->dep_pkg == NULL ? "" : formula->dep_pkg);
    fprintf(installShellScriptFile, "PACKAGE_DEP_UPP='%s'\n", formula->dep_upp == NULL ? "" : formula->dep_upp);
    fprintf(installShellScriptFile, "PACKAGE_DEP_PYM='%s'\n", formula->dep_pym == NULL ? "" : formula->dep_pym);
    fprintf(installShellScriptFile, "PACKAGE_DEP_PLM='%s'\n", formula->dep_plm == NULL ? "" : formula->dep_plm);

    fprintf(installShellScriptFile, "PACKAGE_BSYSTEM='%s'\n", formula->bsystem == NULL ? "" : formula->bsystem);
    fprintf(installShellScriptFile, "PACKAGE_BSCRIPT='%s'\n", formula->bscript == NULL ? "" : formula->bscript);

    fprintf(installShellScriptFile, "PACKAGE_CDEFINE='%s'\n", formula->cdefine == NULL ? "" : formula->cdefine);
    fprintf(installShellScriptFile, "PACKAGE_CCFLAGS='%s'\n", formula->ccflags == NULL ? "" : formula->ccflags);
    fprintf(installShellScriptFile, "PACKAGE_XXFLAGS='%s'\n", formula->xxflags == NULL ? "" : formula->xxflags);
    fprintf(installShellScriptFile, "PACKAGE_LDFLAGS='%s'\n", formula->ldflags == NULL ? "" : formula->ldflags);

    fprintf(installShellScriptFile, "PACKAGE_BINBSTD='%s'\n", formula->binbstd ? "yes" : "no");
    fprintf(installShellScriptFile, "PACKAGE_SYMLINK='%s'\n", formula->symlink ? "yes" : "no");
    fprintf(installShellScriptFile, "PACKAGE_PARALLEL='%s'\n", formula->parallel ? "yes" : "no");

    fprintf(installShellScriptFile, "PACKAGE_FORMULA_FILEPATH='%s'\n\n", formula->path);

    if (formula->prepare == NULL) {
        fprintf(installShellScriptFile, "unset -f prepare\n");
    } else {
        fprintf(installShellScriptFile, "prepare(){\n%s\n}\n", formula->prepare);
    }

    if (formula->install == NULL) {
        fprintf(installShellScriptFile, "unset -f install\n");
    } else {
        fprintf(installShellScriptFile, "install(){\n%s\n}\n", formula->install);
    }



    fprintf(installShellScriptFile, "exit 1\n");
    fprintf(installShellScriptFile, ". /home/leleliu008/ppkg/ppkg-install\n");

    fclose(installShellScriptFile);

    sysinfo_free(sysinfo);
    free(currentExecutablePath);

    //////////////////////////////////////////////////////////////////////////////

    size_t  cmdLength = installShellScriptFilePathLength + 4;
    char    cmd[cmdLength];
    memset (cmd, 0, cmdLength);
    sprintf(cmd, "sh %s", installShellScriptFilePath);

    resultCode = system(cmd);

    if (resultCode != 0) {
        ppkg_formula_free(formula);
        return resultCode;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (!exists_and_is_a_directory(packageInstalledDir)) {
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////

    if (mkdir(packageInstalledMetaInfoDir, S_IRWXU) != 0) {
        perror(packageInstalledMetaInfoDir);
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////

    resultCode = record_installed_files(packageInstalledDir);

    if (resultCode != PPKG_OK) {
        ppkg_formula_free(formula);
        return resultCode;
    }

    //////////////////////////////////////////////////////////////////////

    FILE * receiptFile = fopen(receiptFilePath, "w");

    if (receiptFile == NULL) {
        perror(receiptFilePath);
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    FILE * formulaFile = fopen(formula->path, "r");

    if (formulaFile == NULL) {
        fclose(receiptFile);
        perror(formula->path);
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    ppkg_formula_free(formula);
    formula = NULL;

    fprintf(receiptFile, "pkgname: %s\n", packageName);

    char   buff[1024];
    size_t size = 0;
    while((size = fread(buff, 1, 1024, formulaFile)) != 0) {
        fwrite(buff, 1, size, receiptFile);
    }

    fclose(formulaFile);

    fprintf(receiptFile, "\nsignature: %s\ntimestamp: %lu\n", PPKG_VERSION, time(NULL));

    fclose(receiptFile);

    fprintf(stderr, "package [%s] successfully installed.\n", packageName);

    if (!options.keepInstallingDir) {
        if (rm_r(packageInstallingDir, options.verbose) != 0) {
            perror(packageInstallingDir);
            ppkg_formula_free(formula);
            return PPKG_ERROR;
        }
    }

    return PPKG_OK;
}
