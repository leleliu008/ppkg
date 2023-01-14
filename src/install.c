#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "core/fs.h"
#include "core/log.h"
#include "core/http.h"
#include "core/sysinfo.h"
#include "core/sha256sum.h"
#include "core/base16.h"
#include "core/tar.h"
#include "core/util.h"
#include "core/rm-r.h"
#include "core/find-executables.h"
#include "ppkg.h"

extern int record_installed_files(const char * installedDirPath);

static int ppkg_yy(const char * packageName, PPKGFormula * formula, char **out, size_t * capcity) {
    bool newFormula = false;

    if (formula == NULL) {
        int resultCode = ppkg_formula_parse(packageName, &formula);

        if (resultCode != PPKG_OK) {
            return resultCode;
        }

        newFormula = true;
    }

    if (formula->dep_pkg != NULL) {
        if ((*capcity) == 0) {
            (*capcity) = 256;
            (*out) = (char*)calloc(*capcity, sizeof(char));
        }

        size_t depPackageNamesLength = strlen(formula->dep_pkg);

        if (depPackageNamesLength + strlen(*out) >= (*capcity) - 1) {
            (*capcity) += 256;
            (*out) = (char*)realloc(*out, *capcity);
        }

        strcat(*out, " ");
        strcat(*out, formula->dep_pkg);

        size_t depPackageNamesCopyLength = depPackageNamesLength + 1;
        char   depPackageNamesCopy[depPackageNamesCopyLength];
        memset(depPackageNamesCopy, 0, depPackageNamesCopyLength);
        strcpy(depPackageNamesCopy, formula->dep_pkg);

        char * context;

        char * depPackageName = strtok_r(depPackageNamesCopy, " ", &context);

        while (depPackageName != NULL) {
            int resultCode = ppkg_yy(depPackageName, NULL, out, capcity);

            if (resultCode != PPKG_OK) {
                if (newFormula) {
                    ppkg_formula_free(formula);
                }
                return resultCode;
            }

            depPackageName = strtok_r(NULL, " ", &context);
        }
    }

    if (newFormula) {
        ppkg_formula_free(formula);
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
        ppkg_formula_free(formula);
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        ppkg_formula_free(formula);
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t  ppkgHomeDirLength = userHomeDirLength + 7;
    char    ppkgHomeDir[ppkgHomeDirLength];
    memset (ppkgHomeDir, 0, ppkgHomeDirLength);
    snprintf(ppkgHomeDir, ppkgHomeDirLength, "%s/.ppkg", userHomeDir);

    //////////////////////////////////////////////////////////////////////////////

    size_t  packageInstalledDirLength = userHomeDirLength + packageNameLength + 20;
    char    packageInstalledDir[packageInstalledDirLength];
    memset (packageInstalledDir, 0, packageInstalledDirLength);
    snprintf(packageInstalledDir, packageInstalledDirLength, "%s/.ppkg/installed/%s", userHomeDir, packageName);

    size_t  packageInstalledMetaInfoDirLength = packageInstalledDirLength + 6;
    char    packageInstalledMetaInfoDir[packageInstalledMetaInfoDirLength];
    memset (packageInstalledMetaInfoDir, 0, packageInstalledMetaInfoDirLength);
    snprintf(packageInstalledMetaInfoDir, packageInstalledMetaInfoDirLength, "%s/.ppkg", packageInstalledDir);

    size_t  receiptFilePathLength = packageInstalledMetaInfoDirLength + 12;
    char    receiptFilePath[receiptFilePathLength];
    memset (receiptFilePath, 0, receiptFilePathLength);
    snprintf(receiptFilePath, receiptFilePathLength, "%s/receipt.yml", packageInstalledMetaInfoDir);

    if (exists_and_is_a_regular_file(receiptFilePath)) {
        fprintf(stderr, "package [%s] already has been installed.\n", packageName);
        ppkg_formula_free(formula);
        return PPKG_OK;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t  ppkgInstallingDirLength = ppkgHomeDirLength + packageNameLength + 12;
    char    ppkgInstallingDir[ppkgInstallingDirLength];
    memset (ppkgInstallingDir, 0, ppkgInstallingDirLength);
    snprintf(ppkgInstallingDir, ppkgInstallingDirLength, "%s/installing", ppkgHomeDir);

    if (!exists_and_is_a_directory(ppkgInstallingDir)) {
        if (mkdir(ppkgInstallingDir, S_IRWXU) != 0) {
            perror(ppkgInstallingDir);
            ppkg_formula_free(formula);
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t  packageInstallingTopDirLength = ppkgInstallingDirLength + packageNameLength + 2;
    char    packageInstallingTopDir[packageInstallingTopDirLength];
    memset (packageInstallingTopDir, 0, packageInstallingTopDirLength);
    snprintf(packageInstallingTopDir, packageInstallingTopDirLength, "%s/%s", ppkgInstallingDir, packageName);

    if (exists_and_is_a_directory(packageInstallingTopDir)) {
        if (rm_r(packageInstallingTopDir, options.verbose) != 0) {
            perror(packageInstallingTopDir);
            ppkg_formula_free(formula);
            return PPKG_ERROR;
        }
    }

    if (mkdir(packageInstallingTopDir, S_IRWXU) != 0) {
        perror(packageInstallingTopDir);
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t  packageInstallingSrcDirLength = packageInstallingTopDirLength + 5;
    char    packageInstallingSrcDir[packageInstallingSrcDirLength];
    memset (packageInstallingSrcDir, 0, packageInstallingSrcDirLength);
    snprintf(packageInstallingSrcDir, packageInstallingSrcDirLength, "%s/src", packageInstallingTopDir);

    if (mkdir(packageInstallingSrcDir, S_IRWXU) != 0) {
        perror(packageInstallingSrcDir);
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t  packageInstallingFixDirLength = packageInstallingTopDirLength + 5;
    char    packageInstallingFixDir[packageInstallingFixDirLength];
    memset (packageInstallingFixDir, 0, packageInstallingFixDirLength);
    snprintf(packageInstallingFixDir, packageInstallingFixDirLength, "%s/fix", packageInstallingTopDir);

    if (mkdir(packageInstallingFixDir, S_IRWXU) != 0) {
        perror(packageInstallingFixDir);
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t  packageInstallingResDirLength = packageInstallingTopDirLength + 5;
    char    packageInstallingResDir[packageInstallingResDirLength];
    memset (packageInstallingResDir, 0, packageInstallingResDirLength);
    snprintf(packageInstallingResDir, packageInstallingResDirLength, "%s/res", packageInstallingTopDir);

    if (mkdir(packageInstallingResDir, S_IRWXU) != 0) {
        perror(packageInstallingResDir);
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t  packageInstallingBinDirLength = packageInstallingTopDirLength + 5;
    char    packageInstallingBinDir[packageInstallingBinDirLength];
    memset (packageInstallingBinDir, 0, packageInstallingBinDirLength);
    snprintf(packageInstallingBinDir, packageInstallingBinDirLength, "%s/bin", packageInstallingTopDir);

    if (mkdir(packageInstallingBinDir, S_IRWXU) != 0) {
        perror(packageInstallingBinDir);
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t  packageInstallingIncDirLength = packageInstallingTopDirLength + 5;
    char    packageInstallingIncDir[packageInstallingIncDirLength];
    memset (packageInstallingIncDir, 0, packageInstallingIncDirLength);
    snprintf(packageInstallingIncDir, packageInstallingIncDirLength, "%s/inc", packageInstallingTopDir);

    if (mkdir(packageInstallingIncDir, S_IRWXU) != 0) {
        perror(packageInstallingIncDir);
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t  packageInstallingLibDirLength = packageInstallingTopDirLength + 5;
    char    packageInstallingLibDir[packageInstallingLibDirLength];
    memset (packageInstallingLibDir, 0, packageInstallingLibDirLength);
    snprintf(packageInstallingLibDir, packageInstallingLibDirLength, "%s/lib", packageInstallingTopDir);

    if (mkdir(packageInstallingLibDir, S_IRWXU) != 0) {
        perror(packageInstallingLibDir);
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t  packageInstallingTmpDirLength = packageInstallingTopDirLength + 5;
    char    packageInstallingTmpDir[packageInstallingTmpDirLength];
    memset (packageInstallingTmpDir, 0, packageInstallingTmpDirLength);
    snprintf(packageInstallingTmpDir, packageInstallingTmpDirLength, "%s/tmp", packageInstallingTopDir);

    if (mkdir(packageInstallingTmpDir, S_IRWXU) != 0) {
        perror(packageInstallingTmpDir);
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    fprintf(stderr, "prepare to install package [%s].\n", packageName);

    bool cargo = false;

    if (formula->bsystem != NULL) {
        char * bsystem = strtok(formula->bsystem, " ");

        while (bsystem != NULL) {
            if (strcmp(bsystem, "cargo") == 0) {
                cargo = true;
                break;
            }

            bsystem = strtok(NULL, " ");
        }
    }

    if (cargo) {
        size_t  cargoHomeDirLength = userHomeDirLength + 8;
        char    cargoHomeDir[cargoHomeDirLength];
        memset( cargoHomeDir, 0, cargoHomeDirLength);
        snprintf(cargoHomeDir, cargoHomeDirLength, "%s/.cargo", userHomeDir);

        char * CARGO_HOME = getenv("CARGO_HOME");

        if (CARGO_HOME == NULL) {
            if (exists_and_is_a_directory(cargoHomeDir)) {
                setenv("CARGO_HOME", cargoHomeDir, 1);

                char * PATH = getenv("PATH");

                if (PATH == NULL || strcmp(PATH, "") == 0) {
                    ppkg_formula_free(formula);
                    return PPKG_ENV_PATH_NOT_SET;
                }

                size_t  newPATHLength = cargoHomeDirLength + strlen(PATH) + 2;
                char    newPATH[newPATHLength];
                memset( newPATH, 0, newPATHLength);
                snprintf(newPATH, newPATHLength, "%s:%s", cargoHomeDir, PATH);

                setenv("PATH", newPATH, 1);
            }
        }

        ExecuablePathList pathList = {0};

        resultCode = find_executables(&pathList, "rustup", false);

        if (resultCode != 0) {
            ppkg_formula_free(formula);
            return resultCode;
        }

        if (pathList.size == 0) {
            LOG_INFO("rustup command is required, but it is not found on this machine, I will install it via running shell script.");

            size_t  rustupInitScriptFilePathLength = ppkgHomeDirLength + 16;
            char    rustupInitScriptFilePath[rustupInitScriptFilePathLength];
            memset( rustupInitScriptFilePath, 0, rustupInitScriptFilePathLength);
            snprintf(rustupInitScriptFilePath, rustupInitScriptFilePathLength, "%s/rustup-init.sh", ppkgHomeDir);

            if (exists_and_is_a_regular_file(rustupInitScriptFilePath)) {
                if (unlink(rustupInitScriptFilePath) != 0) {
                    perror(rustupInitScriptFilePath);
                    ppkg_formula_free(formula);
                    return PPKG_ERROR;
                }
            }

            if (http_fetch_to_file("https://sh.rustup.rs", rustupInitScriptFilePath, options.verbose, options.verbose) != 0) {
                ppkg_formula_free(formula);
                return PPKG_NETWORK_ERROR;
            }

            size_t  cmdLength = ppkgHomeDirLength + 9;
            char    cmd[cmdLength];
            memset( cmd, 0, cmdLength);
            snprintf(cmd, cmdLength, "bash %s -y", rustupInitScriptFilePath);

            resultCode = system(cmd);

            if (resultCode != 0) {
                perror(cmd);
                ppkg_formula_free(formula);
                return PPKG_ERROR;
            }

            setenv("CARGO_HOME", cargoHomeDir, 1);

            char * PATH = getenv("PATH");

            if (PATH == NULL || strcmp(PATH, "") == 0) {
                ppkg_formula_free(formula);
                return PPKG_ENV_PATH_NOT_SET;
            }

            size_t  newPATHLength = cargoHomeDirLength + 2;
            char    newPATH[newPATHLength];
            memset( newPATH, 0, newPATHLength);
            snprintf(newPATH, newPATHLength, "%s:%s", cargoHomeDir, PATH);

            setenv("PATH", newPATH, 1);
        } else {
            free(pathList.paths[0]);
            pathList.paths[0] = NULL;

            free(pathList.paths);
            pathList.paths = NULL;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t  ppkgDownloadsDirLength = userHomeDirLength + 18;
    char    ppkgDownloadsDir[ppkgDownloadsDirLength];
    memset (ppkgDownloadsDir, 0, ppkgDownloadsDirLength);
    snprintf(ppkgDownloadsDir, ppkgDownloadsDirLength, "%s/.ppkg/downloads", userHomeDir);

    if (!exists_and_is_a_directory(ppkgDownloadsDir)) {
        if (mkdir(ppkgDownloadsDir, S_IRWXU) != 0) {
            perror(ppkgDownloadsDir);
            ppkg_formula_free(formula);
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->src_url == NULL) {
        size_t  gitRepositoryDirLength = ppkgDownloadsDirLength + packageNameLength + 6;
        char    gitRepositoryDir[gitRepositoryDirLength];
        memset (gitRepositoryDir, 0, gitRepositoryDirLength);
        snprintf(gitRepositoryDir, gitRepositoryDirLength, "%s/%s.git", ppkgDownloadsDir, packageName);

        if (!exists_and_is_a_directory(gitRepositoryDir)) {
            if (mkdir(gitRepositoryDir, S_IRWXU) != 0) {
                perror(gitRepositoryDir);
                return PPKG_ERROR;
            }
        }

        if (formula->git_sha == NULL) {
            if (formula->git_ref == NULL) {
                resultCode = ppkg_fetch_via_git(gitRepositoryDir, formula->git_url, "refs/heads/master:refs/remotes/origin/master", "master");
            } else {
                size_t  refspecLength = strlen(formula->git_ref) + 28;
                char    refspec[refspecLength];
                memset (refspec, 0, refspecLength);
                snprintf(refspec, refspecLength, "%s:refs/remotes/origin/master", formula->git_ref);

                resultCode = ppkg_fetch_via_git(gitRepositoryDir, formula->git_url, refspec, "master");
            }
        } else {
            size_t  refspecLength = strlen(formula->git_sha) + 28;
            char    refspec[refspecLength];
            memset (refspec, 0, refspecLength);
            snprintf(refspec, refspecLength, "%s:refs/remotes/origin/master", formula->git_sha);

            resultCode = ppkg_fetch_via_git(gitRepositoryDir, formula->git_url, refspec, formula->git_sha);
        }

        if (resultCode != PPKG_OK) {
            ppkg_formula_free(formula);
            return resultCode;
        }

        size_t  cmdLength = gitRepositoryDirLength + packageInstallingSrcDirLength + 10;
        char    cmd[cmdLength];
        memset (cmd, 0, cmdLength);
        snprintf(cmd, cmdLength, "cp -r %s/. %s", gitRepositoryDir, packageInstallingSrcDir);

        printf("%s\n", cmd);
        resultCode = system(cmd);

        if (resultCode != 0) {
            ppkg_formula_free(formula);
            return PPKG_ERROR;
        }
    } else {
        if (formula->src_is_dir) {
            size_t  cmdLength = strlen(formula->src_url) + packageInstallingSrcDirLength + 10 - 6;
            char    cmd[cmdLength];
            memset (cmd, 0, cmdLength);
            snprintf(cmd, cmdLength, "cp -r %s/. %s", &formula->src_url[6], packageInstallingSrcDir);

            printf("%s\n", cmd);
            resultCode = system(cmd);

            if (resultCode != 0) {
                ppkg_formula_free(formula);
                return PPKG_ERROR;
            }
        } else {
            char srcFileNameExtension[21] = {0};

            if (get_file_extension_from_url(srcFileNameExtension, 20, formula->src_url) < 0) {
                ppkg_formula_free(formula);
                return PPKG_ERROR;
            }

            printf("==========>> srcFileNameExtension = %s\n", srcFileNameExtension);

            size_t  srcFileNameLength = strlen(formula->src_sha) + strlen(srcFileNameExtension) + 1;
            char    srcFileName[srcFileNameLength];
            memset( srcFileName, 0, srcFileNameLength);
            snprintf(srcFileName, srcFileNameLength, "%s%s", formula->src_sha, srcFileNameExtension);

            size_t  srcFilePathLength = ppkgDownloadsDirLength + srcFileNameLength + 1;
            char    srcFilePath[srcFilePathLength];
            memset (srcFilePath, 0, srcFilePathLength);
            snprintf(srcFilePath, srcFilePathLength, "%s/%s", ppkgDownloadsDir, srcFileName);

            bool needFetch = true;

            if (exists_and_is_a_regular_file(srcFilePath)) {
                char actualSHA256SUM[65] = {0};

                resultCode = sha256sum_of_file(actualSHA256SUM, srcFilePath);

                if (resultCode != 0) {
                    ppkg_formula_free(formula);
                    return PPKG_ERROR;
                }

                if (strcmp(actualSHA256SUM, formula->src_sha) == 0) {
                    needFetch = false;

                    if (options.verbose) {
                        fprintf(stderr, "%s already have been fetched.\n", srcFilePath);
                    }
                }
            }

            if (needFetch) {
                if (http_fetch_to_file(formula->src_url, srcFilePath, options.verbose, options.verbose) != 0) {
                    ppkg_formula_free(formula);
                    return PPKG_NETWORK_ERROR;
                }

                char actualSHA256SUM[65] = {0};

                resultCode = sha256sum_of_file(actualSHA256SUM, srcFilePath);

                if (resultCode != 0) {
                    ppkg_formula_free(formula);
                    return PPKG_ERROR;
                }

                if (strcmp(actualSHA256SUM, formula->src_sha) != 0) {
                    ppkg_formula_free(formula);
                    fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", formula->src_sha, actualSHA256SUM);
                    return PPKG_SHA256_MISMATCH;
                }
            }

            if (strcmp(srcFileNameExtension, ".zip") == 0 ||
                strcmp(srcFileNameExtension, ".tgz") == 0 ||
                strcmp(srcFileNameExtension, ".txz") == 0 ||
                strcmp(srcFileNameExtension, ".tlz") == 0 ||
                strcmp(srcFileNameExtension, ".tbz2") == 0) {

                resultCode = tar_extract(packageInstallingSrcDir, srcFilePath, ARCHIVE_EXTRACT_TIME, options.verbose, 1);

                if (resultCode != PPKG_OK) {
                    ppkg_formula_free(formula);
                    return resultCode;
                }
            } else {
                size_t  srcFilePath2Length = packageInstallingSrcDirLength + srcFileNameLength + 1;
                char    srcFilePath2[srcFilePath2Length];
                memset (srcFilePath2, 0, srcFilePath2Length);
                snprintf(srcFilePath2, srcFilePath2Length, "%s/%s", packageInstallingSrcDir, srcFileName);

                if (cp(srcFilePath, srcFilePath2) != 0) {
                    ppkg_formula_free(formula);
                    return resultCode;
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->fix_url != NULL) {
        char fixFileNameExtension[21] = {0};

        if (get_file_extension_from_url(fixFileNameExtension, 20, formula->fix_url) < 0) {
            ppkg_formula_free(formula);
            return PPKG_ERROR;
        }

        printf("==========>> fixFileNameExtension = %s\n", fixFileNameExtension);

        size_t  fixFileNameLength = strlen(formula->fix_sha) + strlen(fixFileNameExtension) + 1;
        char    fixFileName[fixFileNameLength];
        memset( fixFileName, 0, fixFileNameLength);
        snprintf(fixFileName, fixFileNameLength, "%s%s", formula->fix_sha, fixFileNameExtension);

        size_t  fixFilePathLength = ppkgDownloadsDirLength + fixFileNameLength + 1;
        char    fixFilePath[fixFilePathLength];
        memset (fixFilePath, 0, fixFilePathLength);
        snprintf(fixFilePath, fixFilePathLength, "%s/%s", ppkgDownloadsDir, fixFileName);

        bool needFetch = true;

        if (exists_and_is_a_regular_file(fixFilePath)) {
            char actualSHA256SUM[65] = {0};

            resultCode = sha256sum_of_file(actualSHA256SUM, fixFilePath);

            if (resultCode != 0) {
                ppkg_formula_free(formula);
                return PPKG_ERROR;
            }

            if (strcmp(actualSHA256SUM, formula->fix_sha) == 0) {
                needFetch = false;

                if (options.verbose) {
                    fprintf(stderr, "%s already have been fetched.\n", fixFilePath);
                }
            }
        }

        if (needFetch) {
            if (http_fetch_to_file(formula->fix_url, fixFilePath, options.verbose, options.verbose) != 0) {
                ppkg_formula_free(formula);
                return PPKG_NETWORK_ERROR;
            }

            char actualSHA256SUM[65] = {0};

            resultCode = sha256sum_of_file(actualSHA256SUM, fixFilePath);

            if (resultCode != 0) {
                ppkg_formula_free(formula);
                return PPKG_ERROR;
            }

            if (strcmp(actualSHA256SUM, formula->fix_sha) != 0) {
                ppkg_formula_free(formula);
                fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", formula->fix_sha, actualSHA256SUM);
                return PPKG_SHA256_MISMATCH;
            }
        }

        if (strcmp(fixFileNameExtension, ".zip") == 0 ||
            strcmp(fixFileNameExtension, ".tgz") == 0 ||
            strcmp(fixFileNameExtension, ".txz") == 0 ||
            strcmp(fixFileNameExtension, ".tlz") == 0 ||
            strcmp(fixFileNameExtension, ".tbz2") == 0) {

            resultCode = tar_extract(packageInstallingFixDir, fixFilePath, ARCHIVE_EXTRACT_TIME, options.verbose, 1);

            if (resultCode != PPKG_OK) {
                ppkg_formula_free(formula);
                return resultCode;
            }
        } else {
            size_t  fixFilePath2Length = packageInstallingFixDirLength + fixFileNameLength + 1;
            char    fixFilePath2[fixFilePath2Length];
            memset (fixFilePath2, 0, fixFilePath2Length);
            snprintf(fixFilePath2, fixFilePath2Length, "%s/%s", packageInstallingFixDir, fixFileName);

            if (cp(fixFilePath, fixFilePath2) != 0) {
                ppkg_formula_free(formula);
                return resultCode;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->res_url != NULL) {
        char resFileNameExtension[21] = {0};

        if (get_file_extension_from_url(resFileNameExtension, 20, formula->res_url) < 0) {
            ppkg_formula_free(formula);
            return PPKG_ERROR;
        }

        printf("==========>> resFileNameExtension = %s\n", resFileNameExtension);

        size_t  resFileNameLength = strlen(formula->res_sha) + strlen(resFileNameExtension) + 1;
        char    resFileName[resFileNameLength];
        memset( resFileName, 0, resFileNameLength);
        snprintf(resFileName, resFileNameLength, "%s%s", formula->fix_sha, resFileNameExtension);

        size_t  resFilePathLength = ppkgDownloadsDirLength + resFileNameLength + 1;
        char    resFilePath[resFilePathLength];
        memset (resFilePath, 0, resFilePathLength);
        snprintf(resFilePath, resFilePathLength, "%s/%s", ppkgDownloadsDir, resFileName);

        bool needFetch = true;

        if (exists_and_is_a_regular_file(resFilePath)) {
            char actualSHA256SUM[65] = {0};

            resultCode = sha256sum_of_file(actualSHA256SUM, resFilePath);

            if (resultCode != 0) {
                ppkg_formula_free(formula);
                return PPKG_ERROR;
            }

            if (strcmp(actualSHA256SUM, formula->res_sha) == 0) {
                needFetch = false;

                if (options.verbose) {
                    fprintf(stderr, "%s already have been fetched.\n", resFilePath);
                }
            }
        }

        if (needFetch) {
            if (http_fetch_to_file(formula->res_url, resFilePath, options.verbose, options.verbose) != 0) {
                ppkg_formula_free(formula);
                return PPKG_NETWORK_ERROR;
            }

            char actualSHA256SUM[65] = {0};

            resultCode = sha256sum_of_file(actualSHA256SUM, resFilePath);

            if (resultCode != 0) {
                ppkg_formula_free(formula);
                return PPKG_ERROR;
            }

            if (strcmp(actualSHA256SUM, formula->res_sha) != 0) {
                ppkg_formula_free(formula);
                fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", formula->res_sha, actualSHA256SUM);
                return PPKG_SHA256_MISMATCH;
            }
        }

        if (strcmp(resFileNameExtension, ".zip") == 0 ||
            strcmp(resFileNameExtension, ".tgz") == 0 ||
            strcmp(resFileNameExtension, ".txz") == 0 ||
            strcmp(resFileNameExtension, ".tlz") == 0 ||
            strcmp(resFileNameExtension, ".tbz2") == 0) {

            resultCode = tar_extract(packageInstallingResDir, resFilePath, ARCHIVE_EXTRACT_TIME, options.verbose, 1);

            if (resultCode != PPKG_OK) {
                ppkg_formula_free(formula);
                return resultCode;
            }
        } else {
            size_t  resFilePath2Length = packageInstallingResDirLength + resFileNameLength + 1;
            char    resFilePath2[resFilePath2Length];
            memset (resFilePath2, 0, resFilePath2Length);
            snprintf(resFilePath2, resFilePath2Length, "%s/%s", packageInstallingResDir, resFileName);

            if (cp(resFilePath, resFilePath2) != 0) {
                ppkg_formula_free(formula);
                return resultCode;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    char * PACKAGE_DEP_PKG_R = NULL;
    size_t capcity = 0;

    resultCode = ppkg_yy(packageName, formula, &PACKAGE_DEP_PKG_R, &capcity);

    if (resultCode != PPKG_OK) {
        ppkg_formula_free(formula);
        return resultCode;
    }

    //////////////////////////////////////////////////////////////////////////////

    SysInfo sysinfo = {0};

    if (sysinfo_make(&sysinfo) != 0) {
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    char * libcName = NULL;

    switch(sysinfo.libc) {
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

    size_t  buildonYmlFilePathLength = packageInstallingTopDirLength + 13;
    char    buildonYmlFilePath[buildonYmlFilePathLength];
    memset (buildonYmlFilePath, 0, buildonYmlFilePathLength);
    snprintf(buildonYmlFilePath, buildonYmlFilePathLength, "%s/buildon.yml", packageInstallingTopDir);

    FILE *  buildonYmlFile = fopen(buildonYmlFilePath, "w");

    if (buildonYmlFile == NULL) {
        perror(buildonYmlFilePath);
        sysinfo_free(sysinfo);
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    fprintf(buildonYmlFile, "os-arch: %s\nos-kind: %s\nos-type: %s\nos-name: %s\nos-vers: %s\nos-ncpu: %lu\nos-libc: %s\n", sysinfo.arch, sysinfo.kind, sysinfo.type, sysinfo.name, sysinfo.vers, sysinfo.ncpu, libcName);

    fclose(buildonYmlFile);

    //////////////////////////////////////////////////////////////////////////////

    size_t  installShellScriptFilePathLength = packageInstallingTopDirLength + 12;
    char    installShellScriptFilePath[installShellScriptFilePathLength];
    memset (installShellScriptFilePath, 0, installShellScriptFilePathLength);
    snprintf(installShellScriptFilePath, installShellScriptFilePathLength, "%s/install.sh", packageInstallingTopDir);

    FILE *  installShellScriptFile = fopen(installShellScriptFilePath, "w");

    if (installShellScriptFile == NULL) {
        perror(installShellScriptFilePath);
        sysinfo_free(sysinfo);
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    fprintf(installShellScriptFile, "set -e\n\n");

    fprintf(installShellScriptFile, "TIMESTAMP_UNIX='%lu'\n\n", time(NULL));

    fprintf(installShellScriptFile, "NATIVE_OS_NCPU='%lu'\n", sysinfo.ncpu);
    fprintf(installShellScriptFile, "NATIVE_OS_ARCH='%s'\n", sysinfo.arch);
    fprintf(installShellScriptFile, "NATIVE_OS_KIND='%s'\n", sysinfo.kind);
    fprintf(installShellScriptFile, "NATIVE_OS_TYPE='%s'\n", sysinfo.type);
    fprintf(installShellScriptFile, "NATIVE_OS_NAME='%s'\n", sysinfo.name);
    fprintf(installShellScriptFile, "NATIVE_OS_VERS='%s'\n", sysinfo.vers);
    fprintf(installShellScriptFile, "NATIVE_OS_LIBC='%s'\n\n", libcName);

    fprintf(installShellScriptFile, "KEEP_INSTALLING_DIR='%s'\n", options.keepInstallingDir ? "yes" : "no");
    fprintf(installShellScriptFile, "DRYRUN='%s'\n", options.dryrun ? "yes" : "no");
    fprintf(installShellScriptFile, "BEAR_ENABLED='%s'\n", options.enableBear ? "yes" : "no");
    fprintf(installShellScriptFile, "CCACHE_ENABLED='%s'\n", options.enableCcache ? "yes" : "no");
    fprintf(installShellScriptFile, "EXPORT_COMPILE_COMMANDS_JSON='%s'\n", options.exportCompileCommandsJson ? "yes" : "no");
    fprintf(installShellScriptFile, "VERBOSE_LEVEL='%d'\n", options.verbose ? 1 : 1);
    fprintf(installShellScriptFile, "BUILD_TYPE='%s'\n", options.buildType == PPKGBuildType_release ? "release" : "debug");

    size_t njobs;

    if (formula->parallel) {
        if (options.parallelJobsCount > 0) {
            njobs = options.parallelJobsCount;
        } else {
            njobs = sysinfo.ncpu;
        }
    } else {
        njobs = 1;
    }

    fprintf(installShellScriptFile, "BUILD_NJOBS='%lu'\n", njobs);

    switch (options.linkType) {
        case PPKGLinkType_static_only:     fprintf(installShellScriptFile, "LINK_TYPE='%s'\n\n", "static-only");
        case PPKGLinkType_shared_only:     fprintf(installShellScriptFile, "LINK_TYPE='%s'\n\n", "shared-only");
        case PPKGLinkType_static_prefered: fprintf(installShellScriptFile, "LINK_TYPE='%s'\n\n", "static-prefered");
        case PPKGLinkType_shared_prefered: fprintf(installShellScriptFile, "LINK_TYPE='%s'\n\n", "shared-prefered");
    }

    fprintf(installShellScriptFile, "INSTALL_LIB=yes\n\n");

    fprintf(installShellScriptFile, "PPKG_VERSION='%s'\n", PPKG_VERSION);
    fprintf(installShellScriptFile, "PPKG_HOME='%s'\n", ppkgHomeDir);
    fprintf(installShellScriptFile, "PPKG_EXECUTABLE='%s'\n\n", currentExecutablePath);

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

    fprintf(installShellScriptFile, "PACKAGE_DEP_PKG_R='%s'\n", PACKAGE_DEP_PKG_R == NULL ? "" : PACKAGE_DEP_PKG_R);
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

    fprintf(installShellScriptFile, "PACKAGE_INSTALLING_TOP_DIR='%s'\n", packageInstallingTopDir);
    fprintf(installShellScriptFile, "PACKAGE_INSTALLING_SRC_DIR='%s'\n", packageInstallingSrcDir);
    fprintf(installShellScriptFile, "PACKAGE_INSTALLING_FIX_DIR='%s'\n", packageInstallingFixDir);
    fprintf(installShellScriptFile, "PACKAGE_INSTALLING_RES_DIR='%s'\n", packageInstallingResDir);
    fprintf(installShellScriptFile, "PACKAGE_INSTALLING_BIN_DIR='%s'\n", packageInstallingBinDir);
    fprintf(installShellScriptFile, "PACKAGE_INSTALLING_INC_DIR='%s'\n", packageInstallingIncDir);
    fprintf(installShellScriptFile, "PACKAGE_INSTALLING_LIB_DIR='%s'\n", packageInstallingLibDir);
    fprintf(installShellScriptFile, "PACKAGE_INSTALLING_TMP_DIR='%s'\n", packageInstallingTmpDir);

    fprintf(installShellScriptFile, "PACKAGE_INSTALL_DIR='%s'\n",   packageInstalledDir);
    fprintf(installShellScriptFile, "PACKAGE_METAINF_DIR='%s'\n\n", packageInstalledMetaInfoDir);

    if (formula->prepare == NULL) {
        fprintf(installShellScriptFile, "unset -f prepare\n\n");
    } else {
        fprintf(installShellScriptFile, "prepare() {\n%s\n}\n\n", formula->prepare);
    }

    if (formula->install == NULL) {
        fprintf(installShellScriptFile, "unset -f build\n\n");
    } else {
        fprintf(installShellScriptFile, "build() {\n%s\n}\n\n", formula->install);
    }


    //fprintf(installShellScriptFile, "MY_INSTALLED_DIR='%s'\n\n", ppkgInstallingDir);

    //fprintf(installShellScriptFile, "exit 1\n");
    //fprintf(installShellScriptFile, ". /home/leleliu008/ppkg/ppkg-install\n");

    size_t n = strlen(PPKG_INSTALL);

    size_t n2 = n >> 1;

    char ppkgInstallShellScript[n2];

    base16_decode(ppkgInstallShellScript, PPKG_INSTALL, n);

    fwrite(ppkgInstallShellScript, 1, n2, installShellScriptFile);

    fclose(installShellScriptFile);

    sysinfo_free(sysinfo);
    free(currentExecutablePath);

    if (PACKAGE_DEP_PKG_R != NULL) {
        free(PACKAGE_DEP_PKG_R);
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t  cmdLength = installShellScriptFilePathLength + 4;
    char    cmd[cmdLength];
    memset (cmd, 0, cmdLength);
    snprintf(cmd, cmdLength, "sh %s", installShellScriptFilePath);

    resultCode = system(cmd);

    if (resultCode != 0) {
        ppkg_formula_free(formula);
        if (WIFEXITED(resultCode)) {
            return WEXITSTATUS(resultCode);
        } else {
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (!exists_and_is_a_directory(packageInstalledDir)) {
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////

    size_t  cratesTomlFilePathLength = packageInstalledDirLength + 14;
    char    cratesTomlFilePath[cratesTomlFilePathLength];
    memset (cratesTomlFilePath, 0, cratesTomlFilePathLength);
    snprintf(cratesTomlFilePath, cratesTomlFilePathLength, "%s/.crates.toml", packageInstalledDir);

    if (exists_and_is_a_regular_file(cratesTomlFilePath)) {
        if (unlink(cratesTomlFilePath) != 0) {
            perror(cratesTomlFilePath);
            ppkg_formula_free(formula);
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////

    size_t  crates2JsonFilePathLength = packageInstalledDirLength + 15;
    char    crates2JsonFilePath[crates2JsonFilePathLength];
    memset (crates2JsonFilePath, 0, crates2JsonFilePathLength);
    snprintf(crates2JsonFilePath, crates2JsonFilePathLength, "%s/.crates2.json", packageInstalledDir);

    if (exists_and_is_a_regular_file(crates2JsonFilePath)) {
        if (unlink(crates2JsonFilePath) != 0) {
            perror(crates2JsonFilePath);
            ppkg_formula_free(formula);
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////

    if (mkdir(packageInstalledMetaInfoDir, S_IRWXU) != 0) {
        perror(packageInstalledMetaInfoDir);
        ppkg_formula_free(formula);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->dep_pkg != NULL) {
        size_t  dependenciesDotFilePathLength = packageInstallingTopDirLength + 18;
        char    dependenciesDotFilePath[dependenciesDotFilePathLength];
        memset (dependenciesDotFilePath, 0, dependenciesDotFilePathLength);
        snprintf(dependenciesDotFilePath, dependenciesDotFilePathLength, "%s/dependencies.dot", packageInstallingTopDir);

        size_t  dependenciesDotFilePath2Length = packageInstalledMetaInfoDirLength + 18;
        char    dependenciesDotFilePath2[dependenciesDotFilePath2Length];
        memset (dependenciesDotFilePath2, 0, dependenciesDotFilePath2Length);
        snprintf(dependenciesDotFilePath2, dependenciesDotFilePath2Length, "%s/dependencies.dot", packageInstalledMetaInfoDir);

        if (cp(dependenciesDotFilePath, dependenciesDotFilePath2) != 0) {
            ppkg_formula_free(formula);
            return PPKG_ERROR;
        }

        //////////////////////////////////////////////////////////////////////////////

        size_t  dependenciesTxtFilePathLength = packageInstallingTopDirLength + 18;
        char    dependenciesTxtFilePath[dependenciesTxtFilePathLength];
        memset (dependenciesTxtFilePath, 0, dependenciesTxtFilePathLength);
        snprintf(dependenciesTxtFilePath, dependenciesTxtFilePathLength, "%s/dependencies.txt", packageInstallingTopDir);

        if (exists_and_is_a_regular_file(dependenciesTxtFilePath)) {
            size_t  dependenciesTxtFilePath2Length = packageInstalledMetaInfoDirLength + 18;
            char    dependenciesTxtFilePath2[dependenciesTxtFilePath2Length];
            memset (dependenciesTxtFilePath2, 0, dependenciesTxtFilePath2Length);
            snprintf(dependenciesTxtFilePath2, dependenciesTxtFilePath2Length, "%s/dependencies.txt", packageInstalledMetaInfoDir);

            if (cp(dependenciesTxtFilePath, dependenciesTxtFilePath2) != 0) {
                ppkg_formula_free(formula);
                return PPKG_ERROR;
            }
        }

        //////////////////////////////////////////////////////////////////////////////

        size_t  dependenciesSvgFilePathLength = packageInstallingTopDirLength + 18;
        char    dependenciesSvgFilePath[dependenciesSvgFilePathLength];
        memset (dependenciesSvgFilePath, 0, dependenciesSvgFilePathLength);
        snprintf(dependenciesSvgFilePath, dependenciesSvgFilePathLength, "%s/dependencies.svg", packageInstallingTopDir);

        if (exists_and_is_a_regular_file(dependenciesSvgFilePath)) {
            size_t  dependenciesSvgFilePath2Length = packageInstalledMetaInfoDirLength + 18;
            char    dependenciesSvgFilePath2[dependenciesSvgFilePath2Length];
            memset (dependenciesSvgFilePath2, 0, dependenciesSvgFilePath2Length);
            snprintf(dependenciesSvgFilePath2, dependenciesSvgFilePath2Length, "%s/dependencies.svg", packageInstalledMetaInfoDir);

            if (cp(dependenciesSvgFilePath, dependenciesSvgFilePath2) != 0) {
                ppkg_formula_free(formula);
                return PPKG_ERROR;
            }
        }

        //////////////////////////////////////////////////////////////////////////////

        size_t  dependenciesPngFilePathLength = packageInstallingTopDirLength + 18;
        char    dependenciesPngFilePath[dependenciesPngFilePathLength];
        memset (dependenciesPngFilePath, 0, dependenciesPngFilePathLength);
        snprintf(dependenciesPngFilePath, dependenciesPngFilePathLength, "%s/dependencies.png", packageInstallingTopDir);

        if (exists_and_is_a_regular_file(dependenciesPngFilePath)) {
            size_t  dependenciesPngFilePath2Length = packageInstalledMetaInfoDirLength + 18;
            char    dependenciesPngFilePath2[dependenciesPngFilePath2Length];
            memset (dependenciesPngFilePath2, 0, dependenciesPngFilePath2Length);
            snprintf(dependenciesPngFilePath2, dependenciesPngFilePath2Length, "%s/dependencies.png", packageInstalledMetaInfoDir);

            if (cp(dependenciesPngFilePath, dependenciesPngFilePath2) != 0) {
                ppkg_formula_free(formula);
                return PPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (options.exportCompileCommandsJson) {
        size_t  compileCommandsJsonFilePath2Length = packageInstalledMetaInfoDirLength + 23;
        char    compileCommandsJsonFilePath2[compileCommandsJsonFilePath2Length];
        memset (compileCommandsJsonFilePath2, 0, compileCommandsJsonFilePath2Length);
        snprintf(compileCommandsJsonFilePath2, compileCommandsJsonFilePath2Length, "%s/compile_commands.json", packageInstalledMetaInfoDir);

        size_t  compileCommandsJsonFilePathLength = packageInstallingTmpDirLength + 23;
        char    compileCommandsJsonFilePath[compileCommandsJsonFilePathLength];
        memset (compileCommandsJsonFilePath, 0, compileCommandsJsonFilePathLength);
        snprintf(compileCommandsJsonFilePath, compileCommandsJsonFilePathLength, "%s/compile_commands.json", packageInstallingTmpDir);

        if (exists_and_is_a_regular_file(compileCommandsJsonFilePath)) {
            if (cp(compileCommandsJsonFilePath, compileCommandsJsonFilePath2) != 0) {
                ppkg_formula_free(formula);
                return PPKG_ERROR;
            }
        } else {
            if (formula->bscript == NULL) {
                size_t  compileCommandsJsonFilePathLength = packageInstallingSrcDirLength + 23;
                char    compileCommandsJsonFilePath[compileCommandsJsonFilePathLength];
                memset (compileCommandsJsonFilePath, 0, compileCommandsJsonFilePathLength);
                snprintf(compileCommandsJsonFilePath, compileCommandsJsonFilePathLength, "%s/compile_commands.json", packageInstallingSrcDir);

                if (exists_and_is_a_regular_file(compileCommandsJsonFilePath)) {
                    if (cp(compileCommandsJsonFilePath, compileCommandsJsonFilePath2) != 0) {
                        ppkg_formula_free(formula);
                        return PPKG_ERROR;
                    }
                }
            } else {
                size_t  compileCommandsJsonFilePathLength = packageInstallingSrcDirLength + strlen(formula->bscript) + 24;
                char    compileCommandsJsonFilePath[compileCommandsJsonFilePathLength];
                memset (compileCommandsJsonFilePath, 0, compileCommandsJsonFilePathLength);
                snprintf(compileCommandsJsonFilePath, compileCommandsJsonFilePathLength, "%s/%s/compile_commands.json", packageInstallingSrcDir, formula->bscript);

                if (exists_and_is_a_regular_file(compileCommandsJsonFilePath)) {
                    if (cp(compileCommandsJsonFilePath, compileCommandsJsonFilePath2) != 0) {
                        ppkg_formula_free(formula);
                        return PPKG_ERROR;
                    }
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t  buildConfigLogFilePath2Length = packageInstalledMetaInfoDirLength + 12;
    char    buildConfigLogFilePath2[buildConfigLogFilePath2Length];
    memset (buildConfigLogFilePath2, 0, buildConfigLogFilePath2Length);
    snprintf(buildConfigLogFilePath2, buildConfigLogFilePath2Length, "%s/config.log", packageInstalledMetaInfoDir);

    size_t  buildConfigLogFilePathLength = packageInstallingTmpDirLength + 12;
    char    buildConfigLogFilePath[buildConfigLogFilePathLength];
    memset (buildConfigLogFilePath, 0, buildConfigLogFilePathLength);
    snprintf(buildConfigLogFilePath, buildConfigLogFilePathLength, "%s/config.log", packageInstallingTmpDir);

    if (exists_and_is_a_regular_file(buildConfigLogFilePath)) {
        if (cp(buildConfigLogFilePath, buildConfigLogFilePath2) != 0) {
            ppkg_formula_free(formula);
            return PPKG_ERROR;
        }
    } else {
        if (formula->bscript == NULL) {
            size_t  buildConfigLogFilePathLength = packageInstallingSrcDirLength + 12;
            char    buildConfigLogFilePath[buildConfigLogFilePathLength];
            memset (buildConfigLogFilePath, 0, buildConfigLogFilePathLength);
            snprintf(buildConfigLogFilePath, buildConfigLogFilePathLength, "%s/config.log", packageInstallingSrcDir);

            if (exists_and_is_a_regular_file(buildConfigLogFilePath)) {
                if (cp(buildConfigLogFilePath, buildConfigLogFilePath2) != 0) {
                    ppkg_formula_free(formula);
                    return PPKG_ERROR;
                }
            }
        } else {
            size_t  buildConfigLogFilePathLength = packageInstallingSrcDirLength + strlen(formula->bscript) + 13;
            char    buildConfigLogFilePath[buildConfigLogFilePathLength];
            memset (buildConfigLogFilePath, 0, buildConfigLogFilePathLength);
            snprintf(buildConfigLogFilePath, buildConfigLogFilePathLength, "%s/%s/config.log", packageInstallingSrcDir, formula->bscript);

            if (exists_and_is_a_regular_file(buildConfigLogFilePath)) {
                if (cp(buildConfigLogFilePath, buildConfigLogFilePath2) != 0) {
                    ppkg_formula_free(formula);
                    return PPKG_ERROR;
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////

    size_t  buildonYmlFilePath2Length = packageInstalledMetaInfoDirLength + 13;
    char    buildonYmlFilePath2[buildonYmlFilePath2Length];
    memset (buildonYmlFilePath2, 0, buildonYmlFilePath2Length);
    snprintf(buildonYmlFilePath2, buildonYmlFilePath2Length, "%s/buildon.yml", packageInstalledMetaInfoDir);

    if (cp(buildonYmlFilePath, buildonYmlFilePath2) != 0) {
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

    fprintf(receiptFile, "pkgname: %s\n", packageName);

    if (formula->version_is_calculated) {
        fprintf(receiptFile, "version: %s\n", formula->version);
    }

    if (formula->bsystem_is_calculated) {
        fprintf(receiptFile, "bsystem: %s\n", formula->bsystem);
    }

    if (formula->web_url_is_calculated) {
        fprintf(receiptFile, "web-url: %s\n", formula->web_url);
    }

    ppkg_formula_free(formula);
    formula = NULL;

    char   buff[1024];
    size_t size = 0;
    while((size = fread(buff, 1, 1024, formulaFile)) != 0) {
        fwrite(buff, 1, size, receiptFile);
    }

    fclose(formulaFile);

    fprintf(receiptFile, "\nsignature: %s\ntimestamp: %lu\n", PPKG_VERSION, time(NULL));

    fclose(receiptFile);

    LOG_SUCCESS3("package [ ", packageName, " ] successfully installed!");

    return PPKG_OK;
    if (!options.keepInstallingDir) {
        if (rm_r(packageInstallingTopDir, options.verbose) != 0) {
            perror(packageInstallingTopDir);
            ppkg_formula_free(formula);
            return PPKG_ERROR;
        }
    }

    return PPKG_OK;
}
