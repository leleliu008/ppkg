#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <libgen.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "core/cp.h"
#include "core/log.h"
#include "core/http.h"
#include "core/sysinfo.h"
#include "core/sha256sum.h"
#include "core/base16.h"
#include "core/tar.h"
#include "core/exe.h"
#include "core/self.h"
#include "core/rm-r.h"
#include "core/exe.h"

#include "ppkg.h"


#define LOG_STEP(output2Terminal, logLevel, stepN, msg) \
    if (logLevel != AutotoolsSetupLogLevel_silent) { \
        if (output2Terminal) { \
            fprintf(stderr, "\n%s=>> STEP %u : %s%s\n", COLOR_PURPLE, stepN, msg, COLOR_OFF); \
        } else { \
            fprintf(stderr, "\n=>> STEP %u : %s\n", stepN, msg); \
        } \
    }

#define LOG_RUN_CMD(output2Terminal, logLevel, cmd) \
    if (logLevel != AutotoolsSetupLogLevel_silent) { \
        if (output2Terminal) { \
            fprintf(stderr, "%s==>%s %s%s%s\n", COLOR_PURPLE, COLOR_OFF, COLOR_GREEN, cmd, COLOR_OFF); \
        } else { \
            fprintf(stderr, "==> %s\n", cmd); \
        } \
    }

static int run_cmd(char * cmd, int output2FD) {
    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (pid == 0) {
        if (output2FD > 0) {
            if (dup2(output2FD, STDOUT_FILENO) < 0) {
                perror(NULL);
                exit(125);
            }

            if (dup2(output2FD, STDERR_FILENO) < 0) {
                perror(NULL);
                exit(126);
            }
        }

        ////////////////////////////////////////

        size_t argc = 0U;
        char*  argv[15] = {0};

        char * arg = strtok(cmd, " ");

        while (arg != NULL) {
            argv[argc] = arg;
            argc++;
            arg = strtok(NULL, " ");
        }

        ////////////////////////////////////////

        execv(argv[0], argv);
        perror(argv[0]);
        exit(127);
    } else {
        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (childProcessExitStatusCode == 0) {
            return PPKG_OK;
        }

        if (WIFEXITED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '%s' exit with status code: %d\n", cmd, WEXITSTATUS(childProcessExitStatusCode));
        } else if (WIFSIGNALED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '%s' killed by signal: %d\n", cmd, WTERMSIG(childProcessExitStatusCode));
        } else if (WIFSTOPPED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '%s' stopped by signal: %d\n", cmd, WSTOPSIG(childProcessExitStatusCode));
        }

        return PPKG_ERROR;
    }
}


static int setup_rust_toolchain(int stepN, PPKGInstallOptions options, const char * userHomeDir, size_t userHomeDirLength, const char * packageInstallingTmpDir, size_t packageInstallingTmpDirLength) {
    struct stat st;

    size_t   cargoHomeDirLength = userHomeDirLength + 8U;
    char     cargoHomeDir[cargoHomeDirLength];
    snprintf(cargoHomeDir, cargoHomeDirLength, "%s/.cargo", userHomeDir);

    const char * const CARGO_HOME = getenv("CARGO_HOME");

    if (CARGO_HOME == NULL) {
        if (stat(cargoHomeDir, &st) == 0 && S_ISDIR(st.st_mode)) {
            if (setenv("CARGO_HOME", cargoHomeDir, 1) != 0) {
                perror("CARGO_HOME");
                return PPKG_ERROR;
            }

            const char * const PATH = getenv("PATH");

            if (PATH == NULL || PATH[0] == '\0') {
                return PPKG_ERROR_ENV_PATH_NOT_SET;
            }

            size_t   newPATHLength = cargoHomeDirLength + strlen(PATH) + 2U;
            char     newPATH[newPATHLength];
            snprintf(newPATH, newPATHLength, "%s:%s", cargoHomeDir, PATH);

            if (setenv("PATH", newPATH, 1) != 0) {
                perror("PATH");
                return PPKG_ERROR;
            }
        }
    }

    char * rustupCommandPath = NULL;

    int ret = exe_lookup("rustup", &rustupCommandPath, NULL);

    if (ret == 0) {
        free(rustupCommandPath);
        return PPKG_OK;
    }

    if (ret == PPKG_ERROR_EXE_NOT_FOUND) {
        fprintf(stderr, "%s=>> STEP %u : install rust toolchain%s\n", COLOR_PURPLE, stepN++, COLOR_OFF);

        LOG_INFO("rustup command is required, but it is not found on this machine, ppkg will install it via running shell script.");

        size_t   rustupInitScriptFilePathLength = packageInstallingTmpDirLength + 16U;
        char     rustupInitScriptFilePath[rustupInitScriptFilePathLength];
        snprintf(rustupInitScriptFilePath, rustupInitScriptFilePathLength, "%s/rustup-init.sh", packageInstallingTmpDir);

        int ret = http_fetch_to_file("https://sh.rustup.rs", rustupInitScriptFilePath, options.logLevel >= PPKGLogLevel_verbose, options.logLevel >= PPKGLogLevel_verbose);

        if (ret != PPKG_OK) {
            return ret;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (pid == 0) {
            char* argv[4] = { (char*)"bash", rustupInitScriptFilePath, (char*)"-y", NULL };
            execvp(argv[0], argv);
            perror(argv[0]);
            exit(127);
        } else {
            int childProcessExitStatusCode;

            if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            if (childProcessExitStatusCode == 0) {
                return PPKG_OK;
            } else {
                size_t   cmdLength = rustupInitScriptFilePathLength + 9U;
                char     cmd[cmdLength];
                snprintf(cmd, cmdLength, "bash %s -y", rustupInitScriptFilePath);

                if (WIFEXITED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running command '%s' exit with status code: %d\n", cmd, WEXITSTATUS(childProcessExitStatusCode));
                } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running command '%s' killed by signal: %d\n", cmd, WTERMSIG(childProcessExitStatusCode));
                } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running command '%s' stopped by signal: %d\n", cmd, WSTOPSIG(childProcessExitStatusCode));
                }

                return PPKG_ERROR;
            }
        }

        if (setenv("CARGO_HOME", cargoHomeDir, 1) != 0) {
            perror("CARGO_HOME");
            return PPKG_ERROR;;
        }

        const char * const PATH = getenv("PATH");

        if (PATH == NULL || PATH[0] == '\0') {
            return PPKG_ERROR_ENV_PATH_NOT_SET;
        }

        size_t   newPATHLength = cargoHomeDirLength + 2U;
        char     newPATH[newPATHLength];
        snprintf(newPATH, newPATHLength, "%s:%s", cargoHomeDir, PATH);

        if (setenv("PATH", newPATH, 1) != 0) {
            perror("PATH");
            return PPKG_ERROR;
        }
    } else {
        return ret;
    }
}

static int backup_formulas(const char * sessionDir, const char * packageInstalledMetaInfoDir, size_t packageInstalledMetaInfoDirLength) {
    size_t   packageInstalledFormulaDirLength = packageInstalledMetaInfoDirLength + 9U;
    char     packageInstalledFormulaDir[packageInstalledFormulaDirLength];
    snprintf(packageInstalledFormulaDir, packageInstalledFormulaDirLength, "%s/formula", packageInstalledMetaInfoDir);

    if (mkdir(packageInstalledFormulaDir, S_IRWXU) != 0) {
        perror(packageInstalledFormulaDir);
        return PPKG_ERROR;
    }

    DIR * dir = opendir(sessionDir);

    if (dir == NULL) {
        perror(sessionDir);
        return PPKG_ERROR;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return PPKG_OK;
            } else {
                perror(sessionDir);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t fileNameLength = strlen(dir_entry->d_name);

        size_t   fromFilePathLength = strlen(sessionDir) + fileNameLength  + 2U;
        char     fromFilePath[fromFilePathLength];
        snprintf(fromFilePath, fromFilePathLength, "%s/%s", sessionDir, dir_entry->d_name);

        size_t   toFilePathLength = packageInstalledFormulaDirLength + fileNameLength + 2U;
        char     toFilePath[toFilePathLength];
        snprintf(toFilePath, toFilePathLength, "%s/%s", packageInstalledFormulaDir, dir_entry->d_name);

        int ret = copy_file(fromFilePath, toFilePath);

        if (ret != PPKG_OK) {
            closedir(dir);
            return ret;
        }
    }
}

static int generate_manifest_r(const char * dirPath, size_t offset, FILE * installedManifestFile) {
    if (dirPath == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    size_t dirPathLength = strlen(dirPath);

    if (dirPathLength == 0) {
        return PPKG_ERROR_ARG_IS_EMPTY;
    }

    DIR * dir = opendir(dirPath);

    if (dir == NULL) {
        perror(dirPath);
        return PPKG_ERROR;
    }

    int ret = PPKG_OK;

    struct stat st;

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                break;
            } else {
                perror(dirPath);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t   filePathLength = dirPathLength + strlen(dir_entry->d_name) + 2U;
        char     filePath[filePathLength];
        snprintf(filePath, filePathLength, "%s/%s", dirPath, dir_entry->d_name);

        if (stat(filePath, &st) != 0) {
            perror(filePath);
            closedir(dir);
            return PPKG_ERROR;
        }

        if (S_ISDIR(st.st_mode)) {
            fprintf(installedManifestFile, "d|%s/\n", &filePath[offset]);

            ret = generate_manifest_r(filePath, offset, installedManifestFile);

            if (ret != PPKG_OK) {
                closedir(dir);
                return ret;
            }
        } else {
            fprintf(installedManifestFile, "f|%s\n", &filePath[offset]);
        }
    }

    return ret;
}

int generate_manifest(const char * installedDirPath) {
    size_t installedDirLength = strlen(installedDirPath);

    size_t installedManifestFilePathLength = installedDirLength + 20U;
    char   installedManifestFilePath[installedManifestFilePathLength];
    snprintf(installedManifestFilePath, installedManifestFilePathLength, "%s/.ppkg/manifest.txt", installedDirPath);

    FILE * installedManifestFile = fopen(installedManifestFilePath, "w");

    if (installedManifestFile == NULL) {
        perror(installedManifestFilePath);
        return PPKG_ERROR;
    }

    int ret = generate_manifest_r(installedDirPath, installedDirLength + 1, installedManifestFile);

    fclose(installedManifestFile);

    return ret;
}

static int generate_receipt(const char * packageName, PPKGFormula * formula, SysInfo sysinfo, const char * libcName, const char * packageInstalledMetaInfoDir, size_t packageInstalledMetaInfoDirLength) {
    size_t   receiptFilePathLength = packageInstalledMetaInfoDirLength + 12U;
    char     receiptFilePath[receiptFilePathLength];
    snprintf(receiptFilePath, receiptFilePathLength, "%s/receipt.yml", packageInstalledMetaInfoDir);

    FILE *   receiptFile = fopen(receiptFilePath, "w");

    if (receiptFile == NULL) {
        perror(receiptFilePath);
        return PPKG_ERROR;
    }

    FILE * formulaFile = fopen(formula->path, "r");

    if (formulaFile == NULL) {
        perror(formula->path);
        fclose(receiptFile);
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

    char   buff[1024];
    size_t size = 0;

    for (;;) {
        size = fread(buff, 1, 1024, formulaFile);

        if (ferror(formulaFile)) {
            perror(formula->path);
            fclose(formulaFile);
            fclose(receiptFile);
            return PPKG_ERROR;
        }

        if (size > 0) {
            if (fwrite(buff, 1, size, receiptFile) != size || ferror(receiptFile)) {
                perror(receiptFilePath);
                fclose(receiptFile);
                fclose(formulaFile);
                return PPKG_ERROR;
            }
        }

        if (feof(formulaFile)) {
            fclose(formulaFile);
            break;
        }
    }

    fprintf(receiptFile, "\nsignature: %s\ntimestamp: %lu\n\n", PPKG_VERSION, time(NULL));

    fprintf(receiptFile, "build-on:\n    os-arch: %s\n    os-kind: %s\n    os-type: %s\n    os-name: %s\n    os-vers: %s\n    os-ncpu: %u\n    os-libc: %s\n    os-euid: %u\n    os-egid: %u\n", sysinfo.arch, sysinfo.kind, sysinfo.type, sysinfo.name, sysinfo.vers, sysinfo.ncpu, libcName, sysinfo.euid, sysinfo.egid);

    fclose(receiptFile);

    return PPKG_OK;
}

static int install_files_to_metainfo_dir(struct stat st, const char * fromDIR, size_t fromDIRLength, const char * toDIR, size_t toDIRLength, const char * item, size_t itemLength) {
    size_t   fromFilePathLength = fromDIRLength + itemLength + 2U;
    char     fromFilePath[fromFilePathLength];
    snprintf(fromFilePath, fromFilePathLength, "%s/%s", fromDIR, item);

    size_t   toFilePathLength = toDIRLength + itemLength + 2U;
    char     toFilePath[toFilePathLength];
    snprintf(toFilePath, toFilePathLength, "%s/%s", toDIR, item);

    if (stat(fromFilePath, &st) == 0) {
        if (copy_file(fromFilePath, toFilePath) != 0) {
            return PPKG_ERROR;
        }
    }

    return PPKG_OK;
}

static int ppkg_install_package(
        const char * packageName,
        PPKGFormula * formula,
        PPKGInstallOptions options,
        const char * userHomeDir,
        size_t       userHomeDirLength,
        const char * ppkgHomeDir,
        size_t       ppkgHomeDirLength,
        const char * ppkgInstallingDir,
        size_t       ppkgInstallingDirLength,
        const char * sessionDir,
        SysInfo sysinfo,
        const char * libcName,
        const char * currentExecutablePath,
        struct stat st,
        const char * recursiveDependentPackageNamesString) {
    fprintf(stderr, "%s=============== Installing%s %s%s%s %s===============%s\n", COLOR_PURPLE, COLOR_OFF, COLOR_GREEN, packageName, COLOR_OFF, COLOR_PURPLE, COLOR_OFF);

    unsigned int stepN = 1;

    fprintf(stderr, "%s=>> STEP %u : create the working directory%s\n", COLOR_PURPLE, stepN++, COLOR_OFF);

    size_t   packageNameLength = strlen(packageName);

    size_t   packageInstallingTopDirLength = ppkgInstallingDirLength + packageNameLength + 2U;
    char     packageInstallingTopDir[packageInstallingTopDirLength];
    snprintf(packageInstallingTopDir, packageInstallingTopDirLength, "%s/%s", ppkgInstallingDir, packageName);

    if (stat(packageInstallingTopDir, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            if (rm_r(packageInstallingTopDir, options.logLevel >= PPKGLogLevel_verbose) != 0) {
                perror(packageInstallingTopDir);
                return PPKG_ERROR;
            }
        } else {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", packageInstallingTopDir);
            return PPKG_ERROR;
        }
    }

    if (mkdir(packageInstallingTopDir, S_IRWXU) != 0) {
        perror(packageInstallingTopDir);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   packageInstallingSrcDirLength = packageInstallingTopDirLength + 5U;
    char     packageInstallingSrcDir[packageInstallingSrcDirLength];
    snprintf(packageInstallingSrcDir, packageInstallingSrcDirLength, "%s/src", packageInstallingTopDir);

    if (mkdir(packageInstallingSrcDir, S_IRWXU) != 0) {
        perror(packageInstallingSrcDir);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   packageInstallingFixDirLength = packageInstallingTopDirLength + 5U;
    char     packageInstallingFixDir[packageInstallingFixDirLength];
    snprintf(packageInstallingFixDir, packageInstallingFixDirLength, "%s/fix", packageInstallingTopDir);

    if (mkdir(packageInstallingFixDir, S_IRWXU) != 0) {
        perror(packageInstallingFixDir);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   packageInstallingResDirLength = packageInstallingTopDirLength + 5U;
    char     packageInstallingResDir[packageInstallingResDirLength];
    snprintf(packageInstallingResDir, packageInstallingResDirLength, "%s/res", packageInstallingTopDir);

    if (mkdir(packageInstallingResDir, S_IRWXU) != 0) {
        perror(packageInstallingResDir);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   packageInstallingBinDirLength = packageInstallingTopDirLength + 5U;
    char     packageInstallingBinDir[packageInstallingBinDirLength];
    snprintf(packageInstallingBinDir, packageInstallingBinDirLength, "%s/bin", packageInstallingTopDir);

    if (mkdir(packageInstallingBinDir, S_IRWXU) != 0) {
        perror(packageInstallingBinDir);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   packageInstallingIncDirLength = packageInstallingTopDirLength + 9U;
    char     packageInstallingIncDir[packageInstallingIncDirLength];
    snprintf(packageInstallingIncDir, packageInstallingIncDirLength, "%s/include", packageInstallingTopDir);

    if (mkdir(packageInstallingIncDir, S_IRWXU) != 0) {
        perror(packageInstallingIncDir);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   packageInstallingLibDirLength = packageInstallingTopDirLength + 5U;
    char     packageInstallingLibDir[packageInstallingLibDirLength];
    snprintf(packageInstallingLibDir, packageInstallingLibDirLength, "%s/lib", packageInstallingTopDir);

    if (mkdir(packageInstallingLibDir, S_IRWXU) != 0) {
        perror(packageInstallingLibDir);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   packageInstallingTmpDirLength = packageInstallingTopDirLength + 5U;
    char     packageInstallingTmpDir[packageInstallingTmpDirLength];
    snprintf(packageInstallingTmpDir, packageInstallingTmpDirLength, "%s/tmp", packageInstallingTopDir);

    if (mkdir(packageInstallingTmpDir, S_IRWXU) != 0) {
        perror(packageInstallingTmpDir);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    fprintf(stderr, "%s=>> STEP %u : change to the working directory%s\n", COLOR_PURPLE, stepN++, COLOR_OFF);

    if (chdir(packageInstallingTopDir) != 0) {
        perror(packageInstallingTopDir);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    int ret = PPKG_OK;

    //////////////////////////////////////////////////////////////////////////////

    size_t   ppkgCoreDirLength = ppkgHomeDirLength + 6U;
    char     ppkgCoreDir[ppkgCoreDirLength];
    snprintf(ppkgCoreDir, ppkgCoreDirLength, "%s/core", ppkgHomeDir);

    size_t   ppkgCoreBinDirLength = ppkgCoreDirLength + 5U;
    char     ppkgCoreBinDir[ppkgCoreBinDirLength];
    snprintf(ppkgCoreBinDir, ppkgCoreBinDirLength, "%s/bin", ppkgCoreDir);

    size_t   ppkgCoreAclocalDirLength = ppkgCoreDirLength + 15U;
    char     ppkgCoreAclocalDir[ppkgCoreAclocalDirLength];
    snprintf(ppkgCoreAclocalDir, ppkgCoreAclocalDirLength, "%s/share/aclocal", ppkgCoreDir);

    // https://www.gnu.org/software/automake/manual/html_node/Macro-Search-Path.html
    if (setenv("ACLOCAL_PATH", ppkgCoreAclocalDir, 1) != 0) {
        perror("ACLOCAL_PATH");
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemCargo) {
        ret = setup_rust_toolchain(stepN, options, userHomeDir, userHomeDirLength, packageInstallingTmpDir, packageInstallingTmpDirLength);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    bool isOSDarwin = strcmp(sysinfo.kind, "darwin") == 0 ? true : false;

    char dependentToolsStr[200] = {0};

    strncpy(dependentToolsStr, "pkgconf", 7);

    if (formula->useBuildSystemAutogen || formula->useBuildSystemAutotools) {
        strncat(dependentToolsStr, " automake autoconf perl gm4", 27);
    }

    if (formula->useBuildSystemCmake) {
        strncat(dependentToolsStr, " cmake", 6);
    }

    if (formula->useBuildSystemGmake) {
        strncat(dependentToolsStr, " gmake", 6);
    }

    if (formula->useBuildSystemNinja) {
        strncat(dependentToolsStr, " ninja", 6);
    }

    if (formula->useBuildSystemXmake) {
        strncat(dependentToolsStr, " xmake", 6);
    }

    if (formula->useBuildSystemGolang) {
        strncat(dependentToolsStr, " golang", 7);
    }

    if (options.enableCcache) {
        strncat(dependentToolsStr, " ccache", 7);
    }

    if (formula->fix_url != NULL || formula->res_url != NULL) {
        strncat(dependentToolsStr, " patch", 6);
    }

    if (!isOSDarwin) {
        strncat(dependentToolsStr, " patchelf", 9);
    }

    strncat(dependentToolsStr, " bash coreutils findutils gsed gawk grep tree", 45);

    if (formula->useBuildSystemMeson) {
        strncat(dependentToolsStr, " python3", 8);
    }

    if (formula->dep_pym != NULL) {
        strncat(dependentToolsStr, " perl gmake", 11);
    }

    if (formula->dep_upp == NULL) {
        char * p = strdup(dependentToolsStr);

        if (p == NULL) {
            return PPKG_ERROR_MEMORY_ALLOCATE;
        }

        formula->dep_upp = p;
    } else {
        size_t oldLength = strlen(formula->dep_upp);
        size_t extLength = strlen(dependentToolsStr);
        size_t newLength = oldLength + extLength + 2U;

        char * p = (char*)calloc(newLength, sizeof(char));

        if (p == NULL) {
            return PPKG_ERROR_MEMORY_ALLOCATE;
        }

        snprintf(p, newLength, "%s %s", formula->dep_upp, dependentToolsStr);

        free(formula->dep_upp);

        formula->dep_upp = p;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemMeson) {
        if (formula->dep_pym == NULL) {
            char * p = strdup("meson");

            if (p == NULL) {
                return PPKG_ERROR_MEMORY_ALLOCATE;
            }

            formula->dep_pym = p;
        } else {
            size_t oldLength = strlen(formula->dep_pym);
            size_t newLength = oldLength + 7U;

            char * p = (char*)calloc(newLength, sizeof(char));

            if (p == NULL) {
                return PPKG_ERROR_MEMORY_ALLOCATE;
            }

            snprintf(p, newLength, "%s meson", formula->dep_pym);

            free(formula->dep_pym);

            formula->dep_pym = p;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    fprintf(stderr, "%s=>> STEP %u : fetch src/fix/res from server to local cache%s\n", COLOR_PURPLE, stepN++, COLOR_OFF);

    size_t   ppkgDownloadsDirLength = userHomeDirLength + 18U;
    char     ppkgDownloadsDir[ppkgDownloadsDirLength];
    snprintf(ppkgDownloadsDir, ppkgDownloadsDirLength, "%s/.ppkg/downloads", userHomeDir);

    if (stat(ppkgDownloadsDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", ppkgDownloadsDir);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(ppkgDownloadsDir, S_IRWXU) != 0) {
            perror(ppkgDownloadsDir);
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->src_url == NULL) {
        size_t   gitRepositoryDirLength = ppkgDownloadsDirLength + packageNameLength + 6U;
        char     gitRepositoryDir[gitRepositoryDirLength];
        snprintf(gitRepositoryDir, gitRepositoryDirLength, "%s/%s.git", ppkgDownloadsDir, packageName);

        if (stat(gitRepositoryDir, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", gitRepositoryDir);
                return PPKG_ERROR;
            }
        } else {
            if (mkdir(gitRepositoryDir, S_IRWXU) != 0) {
                perror(gitRepositoryDir);
                return PPKG_ERROR;
            }
        }

        if (formula->git_sha == NULL) {
            if (formula->git_ref == NULL) {
                ret = ppkg_fetch_via_git(gitRepositoryDir, formula->git_url, "refs/heads/master:refs/remotes/origin/master", "master");
            } else {
                size_t   refspecLength = strlen(formula->git_ref) + 28U;
                char     refspec[refspecLength];
                snprintf(refspec, refspecLength, "%s:refs/remotes/origin/master", formula->git_ref);

                ret = ppkg_fetch_via_git(gitRepositoryDir, formula->git_url, refspec, "master");
            }
        } else {
            size_t   refspecLength = strlen(formula->git_sha) + 28U;
            char     refspec[refspecLength];
            snprintf(refspec, refspecLength, "%s:refs/remotes/origin/master", formula->git_sha);

            ret = ppkg_fetch_via_git(gitRepositoryDir, formula->git_url, refspec, formula->git_sha);
        }

        if (ret != PPKG_OK) {
            return ret;
        }

        size_t   xLength = gitRepositoryDirLength + + 3U;
        char     x[xLength];
        snprintf(x, xLength, "%s/.", gitRepositoryDir);

        size_t   cmdLength = gitRepositoryDirLength + packageInstallingSrcDirLength + 10U;
        char     cmd[cmdLength];
        snprintf(cmd, cmdLength, "cp -r %s/. %s", gitRepositoryDir, packageInstallingSrcDir);

        if (options.logLevel >= PPKGLogLevel_verbose) {
            printf("%s\n", cmd);
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (pid == 0) {
            char* argv[5] = { (char*)"cp", (char*)"-r", x, packageInstallingSrcDir, NULL };
            execvp(argv[0], argv);
            perror(argv[0]);
            exit(127);
        } else {
            int childProcessExitStatusCode;

            if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            if (childProcessExitStatusCode != 0) {
                if (WIFEXITED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running command '%s' exit with status code: %d\n", cmd, WEXITSTATUS(childProcessExitStatusCode));
                } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running command '%s' killed by signal: %d\n", cmd, WTERMSIG(childProcessExitStatusCode));
                } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running command '%s' stopped by signal: %d\n", cmd, WSTOPSIG(childProcessExitStatusCode));
                }

                return PPKG_ERROR;
            }
        }
    } else {
        if (formula->src_is_dir) {
            char *   srcDIR = &formula->src_url[6];
            size_t   srcDIRLength = strlen(srcDIR);

            size_t   xLength = srcDIRLength + + 3U;
            char     x[xLength];
            snprintf(x, xLength, "%s/.", srcDIR);

            size_t   cmdLength = srcDIRLength + packageInstallingSrcDirLength + 10U;
            char     cmd[cmdLength];
            snprintf(cmd, cmdLength, "cp -r %s/. %s", srcDIR, packageInstallingSrcDir);

            if (options.logLevel >= PPKGLogLevel_verbose) {
                printf("%s\n", cmd);
            }

            pid_t pid = fork();

            if (pid < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            if (pid == 0) {
                char* argv[5] = { (char*)"cp", (char*)"-r", x, packageInstallingSrcDir, NULL };
                execvp(argv[0], argv);
                perror(argv[0]);
                exit(127);
            } else {
                int childProcessExitStatusCode;

                if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
                    perror(NULL);
                    return PPKG_ERROR;
                }

                if (childProcessExitStatusCode != 0) {
                    if (WIFEXITED(childProcessExitStatusCode)) {
                        fprintf(stderr, "running command '%s' exit with status code: %d\n", cmd, WEXITSTATUS(childProcessExitStatusCode));
                    } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                        fprintf(stderr, "running command '%s' killed by signal: %d\n", cmd, WTERMSIG(childProcessExitStatusCode));
                    } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                        fprintf(stderr, "running command '%s' stopped by signal: %d\n", cmd, WSTOPSIG(childProcessExitStatusCode));
                    }

                    return PPKG_ERROR;
                }
            }
        } else {
            char srcFileNameExtension[21] = {0};

            ret = ppkg_examine_file_extension_from_url(srcFileNameExtension, 20, formula->src_url);

            if (ret != PPKG_OK) {
                return ret;
            }

            size_t   srcFileNameLength = strlen(formula->src_sha) + strlen(srcFileNameExtension) + 1U;
            char     srcFileName[srcFileNameLength];
            snprintf(srcFileName, srcFileNameLength, "%s%s", formula->src_sha, srcFileNameExtension);

            size_t   srcFilePathLength = ppkgDownloadsDirLength + srcFileNameLength + 1U;
            char     srcFilePath[srcFilePathLength];
            snprintf(srcFilePath, srcFilePathLength, "%s/%s", ppkgDownloadsDir, srcFileName);

            bool needFetch = true;

            if (stat(srcFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
                char actualSHA256SUM[65] = {0};

                ret = sha256sum_of_file(actualSHA256SUM, srcFilePath);

                if (ret != PPKG_OK) {
                    return ret;
                }

                if (strcmp(actualSHA256SUM, formula->src_sha) == 0) {
                    needFetch = false;

                    if (options.logLevel >= PPKGLogLevel_verbose) {
                        fprintf(stderr, "%s already have been fetched.\n", srcFilePath);
                    }
                }
            }

            if (needFetch) {
                int ret = http_fetch_to_file(formula->src_url, srcFilePath, options.logLevel >= PPKGLogLevel_verbose, options.logLevel >= PPKGLogLevel_verbose);

                if (ret != PPKG_OK) {
                    return ret;
                }

                char actualSHA256SUM[65] = {0};

                ret = sha256sum_of_file(actualSHA256SUM, srcFilePath);

                if (ret != PPKG_OK) {
                    return ret;
                }

                if (strcmp(actualSHA256SUM, formula->src_sha) != 0) {
                    fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", formula->src_sha, actualSHA256SUM);
                    return PPKG_ERROR_SHA256_MISMATCH;
                }
            }

            if (strcmp(srcFileNameExtension, ".zip") == 0 ||
                strcmp(srcFileNameExtension, ".tgz") == 0 ||
                strcmp(srcFileNameExtension, ".txz") == 0 ||
                strcmp(srcFileNameExtension, ".tlz") == 0 ||
                strcmp(srcFileNameExtension, ".tbz2") == 0) {

                ret = tar_extract(packageInstallingSrcDir, srcFilePath, ARCHIVE_EXTRACT_TIME, options.logLevel >= PPKGLogLevel_verbose, 1);

                if (ret != PPKG_OK) {
                    return ret;
                }
            } else {
                size_t   srcFilePath2Length = packageInstallingSrcDirLength + srcFileNameLength + 1U;
                char     srcFilePath2[srcFilePath2Length];
                snprintf(srcFilePath2, srcFilePath2Length, "%s/%s", packageInstallingSrcDir, srcFileName);

                ret = copy_file(srcFilePath, srcFilePath2);

                if (ret != PPKG_OK) {
                    return ret;
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->fix_url != NULL) {
        char fixFileNameExtension[21] = {0};

        ret = ppkg_examine_file_extension_from_url(fixFileNameExtension, 20, formula->fix_url);

        if (ret != PPKG_OK) {
            return ret;
        }

        size_t   fixFileNameLength = strlen(formula->fix_sha) + strlen(fixFileNameExtension) + 1U;
        char     fixFileName[fixFileNameLength];
        snprintf(fixFileName, fixFileNameLength, "%s%s", formula->fix_sha, fixFileNameExtension);

        size_t   fixFilePathLength = ppkgDownloadsDirLength + fixFileNameLength + 1U;
        char     fixFilePath[fixFilePathLength];
        snprintf(fixFilePath, fixFilePathLength, "%s/%s", ppkgDownloadsDir, fixFileName);

        bool needFetch = true;

        if (stat(fixFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            char actualSHA256SUM[65] = {0};

            ret = sha256sum_of_file(actualSHA256SUM, fixFilePath);

            if (ret != PPKG_OK) {
                return ret;
            }

            if (strcmp(actualSHA256SUM, formula->fix_sha) == 0) {
                needFetch = false;

                if (options.logLevel != PPKGLogLevel_silent) {
                    fprintf(stderr, "%s already have been fetched.\n", fixFilePath);
                }
            }
        }

        if (needFetch) {
            int ret = http_fetch_to_file(formula->fix_url, fixFilePath, options.logLevel >= PPKGLogLevel_verbose, options.logLevel >= PPKGLogLevel_verbose);

            if (ret != PPKG_OK) {
                return ret;
            }

            char actualSHA256SUM[65] = {0};

            ret = sha256sum_of_file(actualSHA256SUM, fixFilePath);

            if (ret != PPKG_OK) {
                return ret;
            }

            if (strcmp(actualSHA256SUM, formula->fix_sha) != 0) {
                fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", formula->fix_sha, actualSHA256SUM);
                return PPKG_ERROR_SHA256_MISMATCH;
            }
        }

        if (strcmp(fixFileNameExtension, ".zip") == 0 ||
            strcmp(fixFileNameExtension, ".tgz") == 0 ||
            strcmp(fixFileNameExtension, ".txz") == 0 ||
            strcmp(fixFileNameExtension, ".tlz") == 0 ||
            strcmp(fixFileNameExtension, ".tbz2") == 0) {

            ret = tar_extract(packageInstallingFixDir, fixFilePath, ARCHIVE_EXTRACT_TIME, options.logLevel >= PPKGLogLevel_verbose, 1);

            if (ret != PPKG_OK) {
                return ret;
            }
        } else {
            size_t   fixFilePath2Length = packageInstallingFixDirLength + fixFileNameLength + 1U;
            char     fixFilePath2[fixFilePath2Length];
            snprintf(fixFilePath2, fixFilePath2Length, "%s/%s", packageInstallingFixDir, fixFileName);

            if ((ret = copy_file(fixFilePath, fixFilePath2)) != PPKG_OK) {
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->res_url != NULL) {
        char resFileNameExtension[21] = {0};

        ret = ppkg_examine_file_extension_from_url(resFileNameExtension, 20, formula->res_url);

        if (ret != PPKG_OK) {
            return ret;
        }

        size_t   resFileNameLength = strlen(formula->res_sha) + strlen(resFileNameExtension) + 1U;
        char     resFileName[resFileNameLength];
        snprintf(resFileName, resFileNameLength, "%s%s", formula->fix_sha, resFileNameExtension);

        size_t   resFilePathLength = ppkgDownloadsDirLength + resFileNameLength + 1U;
        char     resFilePath[resFilePathLength];
        snprintf(resFilePath, resFilePathLength, "%s/%s", ppkgDownloadsDir, resFileName);

        bool needFetch = true;

        if (stat(resFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            char actualSHA256SUM[65] = {0};

            ret = sha256sum_of_file(actualSHA256SUM, resFilePath);

            if (ret != 0) {
                return ret;
            }

            if (strcmp(actualSHA256SUM, formula->res_sha) == 0) {
                needFetch = false;

                if (options.logLevel >= PPKGLogLevel_verbose) {
                    fprintf(stderr, "%s already have been fetched.\n", resFilePath);
                }
            }
        }

        if (needFetch) {
            ret = http_fetch_to_file(formula->res_url, resFilePath, options.logLevel >= PPKGLogLevel_verbose, options.logLevel >= PPKGLogLevel_verbose);

            if (ret != PPKG_OK) {
                return ret;
            }

            char actualSHA256SUM[65] = {0};

            ret = sha256sum_of_file(actualSHA256SUM, resFilePath);

            if (ret != PPKG_OK) {
                return ret;
            }

            if (strcmp(actualSHA256SUM, formula->res_sha) != 0) {
                fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", formula->res_sha, actualSHA256SUM);
                return PPKG_ERROR_SHA256_MISMATCH;
            }
        }

        if (strcmp(resFileNameExtension, ".zip") == 0 ||
            strcmp(resFileNameExtension, ".tgz") == 0 ||
            strcmp(resFileNameExtension, ".txz") == 0 ||
            strcmp(resFileNameExtension, ".tlz") == 0 ||
            strcmp(resFileNameExtension, ".tbz2") == 0) {

            ret = tar_extract(packageInstallingResDir, resFilePath, ARCHIVE_EXTRACT_TIME, options.logLevel >= PPKGLogLevel_verbose, 1);

            if (ret != PPKG_OK) {
                return ret;
            }
        } else {
            size_t   resFilePath2Length = packageInstallingResDirLength + resFileNameLength + 1U;
            char     resFilePath2[resFilePath2Length];
            snprintf(resFilePath2, resFilePath2Length, "%s/%s", packageInstallingResDir, resFileName);

            if ((ret = copy_file(resFilePath, resFilePath2)) != PPKG_OK) {
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   packageInstalledDirLength = ppkgHomeDirLength + packageNameLength + 20U;
    char     packageInstalledDir[packageInstalledDirLength];
    snprintf(packageInstalledDir, packageInstalledDirLength, "%s/installed/%s", ppkgHomeDir, packageName);

    size_t   packageInstalledMetaInfoDirLength = packageInstalledDirLength + 6U;
    char     packageInstalledMetaInfoDir[packageInstalledMetaInfoDirLength];
    snprintf(packageInstalledMetaInfoDir, packageInstalledMetaInfoDirLength, "%s/.ppkg", packageInstalledDir);

    //////////////////////////////////////////////////////////////////////////////

    size_t   installShellScriptFilePathLength = packageInstallingTopDirLength + 12U;
    char     installShellScriptFilePath[installShellScriptFilePathLength];
    snprintf(installShellScriptFilePath, installShellScriptFilePathLength, "%s/install.sh", packageInstallingTopDir);

    FILE *   installShellScriptFile = fopen(installShellScriptFilePath, "w");

    if (installShellScriptFile == NULL) {
        perror(installShellScriptFilePath);
        return PPKG_ERROR;
    }

    const char * const PPKG_XTRACE = getenv("PPKG_XTRACE");

    fprintf(installShellScriptFile, "set -x\n");
    fprintf(installShellScriptFile, "set -e\n\n");

    fprintf(installShellScriptFile, "TIMESTAMP_UNIX='%lu'\n\n", time(NULL));

    fprintf(installShellScriptFile, "NATIVE_OS_NCPU='%u'\n", sysinfo.ncpu);
    fprintf(installShellScriptFile, "NATIVE_OS_ARCH='%s'\n", sysinfo.arch);
    fprintf(installShellScriptFile, "NATIVE_OS_KIND='%s'\n", sysinfo.kind);
    fprintf(installShellScriptFile, "NATIVE_OS_TYPE='%s'\n", sysinfo.type);
    fprintf(installShellScriptFile, "NATIVE_OS_CODE='%s'\n", sysinfo.code);
    fprintf(installShellScriptFile, "NATIVE_OS_NAME='%s'\n", sysinfo.name);
    fprintf(installShellScriptFile, "NATIVE_OS_VERS='%s'\n", sysinfo.vers);
    fprintf(installShellScriptFile, "NATIVE_OS_LIBC='%s'\n\n", libcName);
    fprintf(installShellScriptFile, "NATIVE_OS_EUID='%u'\n", geteuid());
    fprintf(installShellScriptFile, "NATIVE_OS_EGID='%u'\n", getegid());

    fprintf(installShellScriptFile, "KEEP_INSTALLING_DIR='%s'\n", options.keepInstallingDir ? "yes" : "no");
    fprintf(installShellScriptFile, "DRYRUN='%s'\n", options.dryrun ? "yes" : "no");
    fprintf(installShellScriptFile, "BEAR_ENABLED='%s'\n", options.enableBear ? "yes" : "no");
    fprintf(installShellScriptFile, "CCACHE_ENABLED='%s'\n", options.enableCcache ? "yes" : "no");
    fprintf(installShellScriptFile, "EXPORT_COMPILE_COMMANDS_JSON='%s'\n", options.exportCompileCommandsJson ? "yes" : "no");
    fprintf(installShellScriptFile, "LOG_LEVEL='%d'\n", options.logLevel);
    fprintf(installShellScriptFile, "BUILD_TYPE='%s'\n", options.buildType == PPKGBuildType_release ? "release" : "debug");

    size_t njobs;

    if (formula->parallel) {
        if (options.parallelJobsCount > 0) {
            njobs = options.parallelJobsCount;
        } else {
            njobs = sysinfo.ncpu;
        }
    } else {
        njobs = 1U;
    }

    fprintf(installShellScriptFile, "BUILD_NJOBS='%lu'\n", njobs);

    switch (options.linkType) {
        case PPKGLinkType_static_only:     fprintf(installShellScriptFile, "LINK_TYPE='static-only'\n\n");     break;
        case PPKGLinkType_shared_only:     fprintf(installShellScriptFile, "LINK_TYPE='shared-only'\n\n");     break;
        case PPKGLinkType_static_prefered: fprintf(installShellScriptFile, "LINK_TYPE='static-prefered'\n\n"); break;
        case PPKGLinkType_shared_prefered: fprintf(installShellScriptFile, "LINK_TYPE='shared-prefered'\n\n"); break;
    }

    fprintf(installShellScriptFile, "INSTALL_LIB='both'\n\n");

    fprintf(installShellScriptFile, "STATIC_LIBRARY_SUFFIX='.a'\n");

    if (isOSDarwin) {
        fprintf(installShellScriptFile, "SHARED_LIBRARY_SUFFIX='.dylib'\n\n");
    } else {
        fprintf(installShellScriptFile, "SHARED_LIBRARY_SUFFIX='.so'\n\n");
    }

    fprintf(installShellScriptFile, "PPKG_VERSION='%s'\n", PPKG_VERSION);
    fprintf(installShellScriptFile, "PPKG='%s'\n\n", currentExecutablePath);
    fprintf(installShellScriptFile, "PPKG_HOME='%s'\n", ppkgHomeDir);
    fprintf(installShellScriptFile, "PPKG_CORE_DIR='%s'\n", ppkgCoreDir);
    fprintf(installShellScriptFile, "PPKG_CORE_BIN_DIR='%s'\n", ppkgCoreBinDir);
    fprintf(installShellScriptFile, "PPKG_DOWNLOADS_DIR='%s'\n", ppkgDownloadsDir);
    fprintf(installShellScriptFile, "PPKG_FORMULA_REPO_ROOT='%s/repos.d'\n", ppkgHomeDir);
    fprintf(installShellScriptFile, "PPKG_PACKAGE_INSTALLED_ROOT='%s/installed'\n", ppkgHomeDir);
    fprintf(installShellScriptFile, "PPKG_PACKAGE_SYMLINKED_ROOT='%s/symlinked'\n", ppkgHomeDir);

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

    fprintf(installShellScriptFile, "RECURSIVE_DEPENDENT_PACKAGE_NAMES='%s'\n", recursiveDependentPackageNamesString == NULL ? "" : recursiveDependentPackageNamesString);
    fprintf(installShellScriptFile, "PACKAGE_DEP_PKG='%s'\n", formula->dep_pkg == NULL ? "" : formula->dep_pkg);
    fprintf(installShellScriptFile, "PACKAGE_DEP_UPP='%s'\n", formula->dep_upp == NULL ? "" : formula->dep_upp);
    fprintf(installShellScriptFile, "PACKAGE_DEP_PYM='%s'\n", formula->dep_pym == NULL ? "" : formula->dep_pym);
    fprintf(installShellScriptFile, "PACKAGE_DEP_PLM='%s'\n", formula->dep_plm == NULL ? "" : formula->dep_plm);

    fprintf(installShellScriptFile, "PACKAGE_BSYSTEM='%s'\n", formula->bsystem == NULL ? "" : formula->bsystem);
    fprintf(installShellScriptFile, "PACKAGE_BSCRIPT='%s'\n", formula->bscript == NULL ? "" : formula->bscript);

    fprintf(installShellScriptFile, "PACKAGE_PPFLAGS='%s'\n", formula->ppflags == NULL ? "" : formula->ppflags);
    fprintf(installShellScriptFile, "PACKAGE_CCFLAGS='%s'\n", formula->ccflags == NULL ? "" : formula->ccflags);
    fprintf(installShellScriptFile, "PACKAGE_XXFLAGS='%s'\n", formula->xxflags == NULL ? "" : formula->xxflags);
    fprintf(installShellScriptFile, "PACKAGE_LDFLAGS='%s'\n", formula->ldflags == NULL ? "" : formula->ldflags);

    fprintf(installShellScriptFile, "PACKAGE_BINBSTD='%s'\n", formula->binbstd ? "yes" : "no");
    fprintf(installShellScriptFile, "PACKAGE_SYMLINK='%s'\n", formula->symlink ? "yes" : "no");
    fprintf(installShellScriptFile, "PACKAGE_PARALLEL='%s'\n", formula->parallel ? "yes" : "no");

    fprintf(installShellScriptFile, "PACKAGE_BUILD_SYSTEM_AUTOGENSH='%s'\n", formula->useBuildSystemAutogen ? "yes" : "no");
    fprintf(installShellScriptFile, "PACKAGE_BUILD_SYSTEM_AUTOTOOLS='%s'\n", formula->useBuildSystemAutotools ? "yes" : "no");
    fprintf(installShellScriptFile, "PACKAGE_BUILD_SYSTEM_CONFIGURE='%s'\n", formula->useBuildSystemConfigure ? "yes" : "no");
    fprintf(installShellScriptFile, "PACKAGE_BUILD_SYSTEM_CMAKE='%s'\n", formula->useBuildSystemCmake ? "yes" : "no");
    fprintf(installShellScriptFile, "PACKAGE_BUILD_SYSTEM_XMAKE='%s'\n", formula->useBuildSystemXmake ? "yes" : "no");
    fprintf(installShellScriptFile, "PACKAGE_BUILD_SYSTEM_GMAKE='%s'\n", formula->useBuildSystemGmake ? "yes" : "no");
    fprintf(installShellScriptFile, "PACKAGE_BUILD_SYSTEM_NINJA='%s'\n", formula->useBuildSystemNinja ? "yes" : "no");
    fprintf(installShellScriptFile, "PACKAGE_BUILD_SYSTEM_MESON='%s'\n", formula->useBuildSystemMeson ? "yes" : "no");
    fprintf(installShellScriptFile, "PACKAGE_BUILD_SYSTEM_CARGO='%s'\n", formula->useBuildSystemCargo ? "yes" : "no");
    fprintf(installShellScriptFile, "PACKAGE_BUILD_SYSTEM_GO='%s'\n",    formula->useBuildSystemGolang ? "yes" : "no");

    fprintf(installShellScriptFile, "PACKAGE_BUILD_IN_BSCRIPT_DIR='%s'\n", formula->binbstd ? "yes" : "no");

    fprintf(installShellScriptFile, "PACKAGE_FORMULA_FILEPATH='%s'\n\n", formula->path);

    fprintf(installShellScriptFile, "PACKAGE_INSTALLING_SRC_DIR='%s'\n", packageInstallingSrcDir);
    fprintf(installShellScriptFile, "PACKAGE_INSTALLING_FIX_DIR='%s'\n", packageInstallingFixDir);
    fprintf(installShellScriptFile, "PACKAGE_INSTALLING_RES_DIR='%s'\n", packageInstallingResDir);
    fprintf(installShellScriptFile, "PACKAGE_INSTALLING_BIN_DIR='%s'\n", packageInstallingBinDir);
    fprintf(installShellScriptFile, "PACKAGE_INSTALLING_INC_DIR='%s'\n", packageInstallingIncDir);
    fprintf(installShellScriptFile, "PACKAGE_INSTALLING_LIB_DIR='%s'\n", packageInstallingLibDir);
    fprintf(installShellScriptFile, "PACKAGE_INSTALLING_TMP_DIR='%s'\n\n", packageInstallingTmpDir);

    fprintf(installShellScriptFile, "PACKAGE_WORKING_DIR='%s'\n",   packageInstallingTopDir);
    fprintf(installShellScriptFile, "PACKAGE_BCACHED_DIR='%s/_'\n", packageInstallingSrcDir);

    if (formula->bscript == NULL) {
        fprintf(installShellScriptFile, "PACKAGE_BSCRIPT_DIR='%s'\n",    packageInstallingSrcDir);
    } else {
        fprintf(installShellScriptFile, "PACKAGE_BSCRIPT_DIR='%s/%s'\n", packageInstallingSrcDir, formula->bscript);
    }

    fprintf(installShellScriptFile, "PACKAGE_INSTALL_DIR='%s'\n",   packageInstalledDir);
    fprintf(installShellScriptFile, "PACKAGE_METAINF_DIR='%s'\n\n", packageInstalledMetaInfoDir);

    fprintf(installShellScriptFile, "SESSION_DIR='%s'\n\n", sessionDir);

    if (formula->dopatch == NULL) {
        fprintf(installShellScriptFile, "unset -f dopatch\n\n");
    } else {
        fprintf(installShellScriptFile, "dopatch() {\n%s\n}\n\n", formula->dopatch);
    }

    fprintf(installShellScriptFile, "dobuild() {\n%s\n}\n\n", formula->install);

    if (PPKG_XTRACE == NULL || PPKG_XTRACE[0] == '\0') {
        fprintf(installShellScriptFile, "set +x\n");
    }

    size_t n = strlen(PPKG_INSTALL);

    size_t n2 = n >> 1;

    char ppkgInstallShellScript[n2];

    base16_decode(ppkgInstallShellScript, PPKG_INSTALL, n);

    fwrite(ppkgInstallShellScript, 1, n2, installShellScriptFile);

    fclose(installShellScriptFile);

    //////////////////////////////////////////////////////////////////////////////

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (pid == 0) {
        char* argv[3] = { (char*)"sh", installShellScriptFilePath, NULL };
        execvp(argv[0], argv);
        perror(argv[0]);
        exit(127);
    } else {
        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (childProcessExitStatusCode != 0) {
            size_t   cmdLength = installShellScriptFilePathLength + 4U;
            char     cmd[cmdLength];
            snprintf(cmd, cmdLength, "sh %s", installShellScriptFilePath);

            if (WIFEXITED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command '%s' exit with status code: %d\n", cmd, WEXITSTATUS(childProcessExitStatusCode));
            } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command '%s' killed by signal: %d\n", cmd, WTERMSIG(childProcessExitStatusCode));
            } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command '%s' stopped by signal: %d\n", cmd, WSTOPSIG(childProcessExitStatusCode));
            }

            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (stat(packageInstalledDir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "nothing is installed.\n");
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   cratesTomlFilePathLength = packageInstalledDirLength + 14U;
    char     cratesTomlFilePath[cratesTomlFilePathLength];
    snprintf(cratesTomlFilePath, cratesTomlFilePathLength, "%s/.crates.toml", packageInstalledDir);

    if (stat(cratesTomlFilePath, &st) == 0) {
        if (unlink(cratesTomlFilePath) != 0) {
            perror(cratesTomlFilePath);
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   crates2JsonFilePathLength = packageInstalledDirLength + 15U;
    char     crates2JsonFilePath[crates2JsonFilePathLength];
    snprintf(crates2JsonFilePath, crates2JsonFilePathLength, "%s/.crates2.json", packageInstalledDir);

    if (stat(crates2JsonFilePath, &st) == 0) {
        if (unlink(crates2JsonFilePath) != 0) {
            perror(crates2JsonFilePath);
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (mkdir(packageInstalledMetaInfoDir, S_IRWXU) != 0) {
        perror(packageInstalledMetaInfoDir);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////
    // install dependency graph files

    if (formula->dep_pkg != NULL) {
        const char* t[4] = { "dot", "box", "svg", "png" };

        for (int i = 0; i < 4; i++) {
            size_t   itemLength = strlen(t[i]) + 14U;
            char     item[itemLength];
            snprintf(item, itemLength, "dependencies.%s", t[i]);

            ret = install_files_to_metainfo_dir(st, packageInstallingTopDir, packageInstallingTopDirLength, packageInstalledMetaInfoDir, packageInstalledMetaInfoDirLength, item, itemLength);

            if (ret != PPKG_OK) {
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // install compile_commands.json

    if (options.exportCompileCommandsJson) {
        size_t   compileCommandsJsonFilePath2Length = packageInstalledMetaInfoDirLength + 23U;
        char     compileCommandsJsonFilePath2[compileCommandsJsonFilePath2Length];
        snprintf(compileCommandsJsonFilePath2, compileCommandsJsonFilePath2Length, "%s/compile_commands.json", packageInstalledMetaInfoDir);

        size_t   compileCommandsJsonFilePathLength = packageInstallingTmpDirLength + 23U;
        char     compileCommandsJsonFilePath[compileCommandsJsonFilePathLength];
        snprintf(compileCommandsJsonFilePath, compileCommandsJsonFilePathLength, "%s/compile_commands.json", packageInstallingTmpDir);

        if (stat(compileCommandsJsonFilePath, &st) == 0) {
            if (copy_file(compileCommandsJsonFilePath, compileCommandsJsonFilePath2) != 0) {
                return PPKG_ERROR;
            }
        } else {
            if (formula->bscript == NULL) {
                size_t   compileCommandsJsonFilePathLength = packageInstallingSrcDirLength + 23U;
                char     compileCommandsJsonFilePath[compileCommandsJsonFilePathLength];
                snprintf(compileCommandsJsonFilePath, compileCommandsJsonFilePathLength, "%s/compile_commands.json", packageInstallingSrcDir);

                if (stat(compileCommandsJsonFilePath, &st) == 0) {
                    if (copy_file(compileCommandsJsonFilePath, compileCommandsJsonFilePath2) != 0) {
                        return PPKG_ERROR;
                    }
                }
            } else {
                size_t   compileCommandsJsonFilePathLength = packageInstallingSrcDirLength + strlen(formula->bscript) + 24U;
                char     compileCommandsJsonFilePath[compileCommandsJsonFilePathLength];
                snprintf(compileCommandsJsonFilePath, compileCommandsJsonFilePathLength, "%s/%s/compile_commands.json", packageInstallingSrcDir, formula->bscript);

                if (stat(compileCommandsJsonFilePath, &st) == 0) {
                    if (copy_file(compileCommandsJsonFilePath, compileCommandsJsonFilePath2) != 0) {
                        return PPKG_ERROR;
                    }
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // install config.log

    size_t   buildConfigLogFilePath2Length = packageInstalledMetaInfoDirLength + 12U;
    char     buildConfigLogFilePath2[buildConfigLogFilePath2Length];
    snprintf(buildConfigLogFilePath2, buildConfigLogFilePath2Length, "%s/config.log", packageInstalledMetaInfoDir);

    size_t   buildConfigLogFilePathLength = packageInstallingTmpDirLength + 12U;
    char     buildConfigLogFilePath[buildConfigLogFilePathLength];
    snprintf(buildConfigLogFilePath, buildConfigLogFilePathLength, "%s/config.log", packageInstallingTmpDir);

    if (stat(buildConfigLogFilePath, &st) == 0) {
        if (copy_file(buildConfigLogFilePath, buildConfigLogFilePath2) != 0) {
            return PPKG_ERROR;
        }
    } else {
        if (formula->bscript == NULL) {
            size_t  buildConfigLogFilePathLength = packageInstallingSrcDirLength + 12U;
            char    buildConfigLogFilePath[buildConfigLogFilePathLength];
            snprintf(buildConfigLogFilePath, buildConfigLogFilePathLength, "%s/config.log", packageInstallingSrcDir);

            if (stat(buildConfigLogFilePath, &st) == 0) {
                if (copy_file(buildConfigLogFilePath, buildConfigLogFilePath2) != 0) {
                    return PPKG_ERROR;
                }
            }
        } else {
            size_t  buildConfigLogFilePathLength = packageInstallingSrcDirLength + strlen(formula->bscript) + 13U;
            char    buildConfigLogFilePath[buildConfigLogFilePathLength];
            snprintf(buildConfigLogFilePath, buildConfigLogFilePathLength, "%s/%s/config.log", packageInstallingSrcDir, formula->bscript);

            if (stat(buildConfigLogFilePath, &st) == 0) {
                if (copy_file(buildConfigLogFilePath, buildConfigLogFilePath2) != 0) {
                    return PPKG_ERROR;
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    
    const char* a[12] = { "AUTHORS", "LICENSE", "COPYING", "FAQ", "TODO", "NEWS", "THANKS", "CHANGELOG", "CHANGES", "README", "CONTRIBUTORS", "CONTRIBUTING" };

    const char* b[3] = { "", ".md", ".rst" };

    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 3; j++) {
            size_t   itemLength = strlen(a[i]) + strlen(b[j]) + 1U;
            char     item[itemLength];
            snprintf(item, itemLength, "%s%s", a[i], b[j]);

            ret = install_files_to_metainfo_dir(st, packageInstallingSrcDir, packageInstallingSrcDirLength, packageInstalledMetaInfoDir, packageInstalledMetaInfoDirLength, item, itemLength);

            if (ret != PPKG_OK) {
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // install relevant formulas

    ret = backup_formulas(sessionDir, packageInstalledMetaInfoDir, packageInstalledMetaInfoDirLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////
    // generate manifest.txt

    ret = generate_manifest(packageInstalledDir);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////
    // generate receipt.yml

    ret = generate_receipt(packageName, formula, sysinfo, libcName, packageInstalledMetaInfoDir, packageInstalledMetaInfoDirLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    fprintf(stderr, "\n%spackage '%s' successfully installed.%s\n\n", COLOR_GREEN, packageName, COLOR_OFF);

    //////////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////////

    if (!options.keepInstallingDir) {
        if ((ret = rm_r(packageInstallingTopDir, options.logLevel >= PPKGLogLevel_verbose)) != PPKG_OK) {
            perror(packageInstallingTopDir);
            return ret;
        }
    }

    return PPKG_OK;
}

static int tree_installed_files(char * packageInstalledDir, size_t packageInstalledDirLength) {
    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (pid == 0) {
        char* argv[5] = { (char*)"tree", (char*)"-a", (char*)"--dirsfirst", packageInstalledDir, NULL };
        execvp(argv[0], argv);
        perror(argv[0]);
        exit(127);
    } else {
        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (childProcessExitStatusCode == 0) {
            return PPKG_OK;
        } else {
            size_t   cmdLength = packageInstalledDirLength + 21U;
            char     cmd[cmdLength];
            snprintf(cmd, cmdLength, "tree -a --dirsfirst %s", packageInstalledDir);

            if (WIFEXITED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command '%s' exit with status code: %d\n", cmd, WEXITSTATUS(childProcessExitStatusCode));
            } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command '%s' killed by signal: %d\n", cmd, WTERMSIG(childProcessExitStatusCode));
            } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command '%s' stopped by signal: %d\n", cmd, WSTOPSIG(childProcessExitStatusCode));
            }

            return PPKG_ERROR;
        }
    }
}

typedef struct {
    char * packageName;
    PPKGFormula * formula;
} PPKGPackage;

typedef struct {
    char ** array;
    size_t  size;
    size_t  capcity;
} RecursiveDependentPackageNameArrayList;

static int getRecursiveDependentPackageNameArrayList(char * packageName, PPKGPackage ** packageSet, size_t packageSetSize, char *** precursiveDependentPackageNameArrayList, size_t * precursiveDependentPackageNameArrayListSize, size_t * precursiveDependentPackageNameArrayListCapcity) {
    char * * recursiveDependentPackageNameArrayList        = (*precursiveDependentPackageNameArrayList);
    size_t   recursiveDependentPackageNameArrayListSize    = (*precursiveDependentPackageNameArrayListSize);
    size_t   recursiveDependentPackageNameArrayListCapcity = (*precursiveDependentPackageNameArrayListCapcity);


    size_t   packageNameStackCapcity = 10;
    size_t   packageNameStackSize    = 1;
    char * * packageNameStack = (char**)malloc(10 * sizeof(char*));

    if (packageNameStack == NULL) {
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    packageNameStack[0] = packageName;

    ////////////////////////////////////////////////////////////////

    while (packageNameStackSize > 0) {
        char * packageName = packageNameStack[packageNameStackSize - 1];
        packageNameStack[packageNameStackSize - 1] = NULL;
        packageNameStackSize--;

        ////////////////////////////////////////////////////////////////

        bool alreadyInRecursiveDependentPackageNameArrayList = false;

        for (size_t i = 0; i < recursiveDependentPackageNameArrayListSize; i++) {
            if (strcmp(recursiveDependentPackageNameArrayList[i], packageName) == 0) {
                alreadyInRecursiveDependentPackageNameArrayList = true;
                break;
            }
        }

        if (alreadyInRecursiveDependentPackageNameArrayList) {
            continue;
        }

        ////////////////////////////////////////////////////////////////

        if (recursiveDependentPackageNameArrayListSize == recursiveDependentPackageNameArrayListCapcity) {
            char ** p = (char**)realloc(recursiveDependentPackageNameArrayList, (recursiveDependentPackageNameArrayListCapcity + 10) * sizeof(char*));

            if (p == NULL) {
                free(recursiveDependentPackageNameArrayList);
                recursiveDependentPackageNameArrayList = NULL;

                free(packageNameStack);
                packageNameStack = NULL;

                return PPKG_ERROR_MEMORY_ALLOCATE;
            }

            recursiveDependentPackageNameArrayList = p;
            recursiveDependentPackageNameArrayListCapcity += 10;
        }

        recursiveDependentPackageNameArrayList[recursiveDependentPackageNameArrayListSize] = packageName;
        recursiveDependentPackageNameArrayListSize++;

        ////////////////////////////////////////////////////////////////

        PPKGPackage * package = NULL;

        for (size_t i = 0; i < packageSetSize; i++) {
            if (strcmp(packageSet[i]->packageName, packageName) == 0) {
                package = packageSet[i];
                break;
            }
        }

        ////////////////////////////////////////////////////////////////

        PPKGFormula * formula = package->formula;

        if (formula->dep_pkg != NULL) {
            size_t depPackageNamesLength = strlen(formula->dep_pkg);

            size_t depPackageNamesCopyLength = depPackageNamesLength + 1U;
            char   depPackageNamesCopy[depPackageNamesCopyLength];
            strncpy(depPackageNamesCopy, formula->dep_pkg, depPackageNamesCopyLength);

            char * depPackageName = strtok(depPackageNamesCopy, " ");

            while (depPackageName != NULL) {
                PPKGPackage * depPackage = NULL;

                for (size_t i = 0; i < packageSetSize; i++) {
                    if (strcmp(packageSet[i]->packageName, depPackageName) == 0) {
                        depPackage = packageSet[i];
                        break;
                    }
                }

                ////////////////////////////////////////////////////////////////

                if (packageNameStackSize == packageNameStackCapcity) {
                    char ** p = (char**)realloc(packageNameStack, (packageNameStackCapcity + 10) * sizeof(char*));

                    if (p == NULL) {
                        free(recursiveDependentPackageNameArrayList);
                        recursiveDependentPackageNameArrayList = NULL;

                        free(packageNameStack);
                        packageNameStack = NULL;

                        return PPKG_ERROR_MEMORY_ALLOCATE;
                    }

                    packageNameStack = p;
                    packageNameStackCapcity += 10;
                }

                packageNameStack[packageNameStackSize] = depPackage->packageName;
                packageNameStackSize++;

                ////////////////////////////////////////////////////////////////

                depPackageName = strtok (NULL, " ");
            }
        }
    }

    free(packageNameStack);

    (*precursiveDependentPackageNameArrayList)        = recursiveDependentPackageNameArrayList;
    (*precursiveDependentPackageNameArrayListSize)    = recursiveDependentPackageNameArrayListSize;
    (*precursiveDependentPackageNameArrayListCapcity) = recursiveDependentPackageNameArrayListCapcity;

    return PPKG_OK;
}

static int check_and_read_formula_in_cache(const char * packageName, const char * sessionDir, PPKGPackage *** ppackageSet, size_t * ppackageSetSize, size_t * ppackageSetCapcity) {
    size_t         packageSetCapcity = (*ppackageSetCapcity);
    size_t         packageSetSize    = (*ppackageSetSize);
    PPKGPackage ** packageSet        = (*ppackageSet);

    size_t   packageNameStackCapcity = 10;
    size_t   packageNameStackSize    = 0;
    char * * packageNameStack = (char**)malloc(10 * sizeof(char*));

    if (packageNameStack == NULL) {
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    packageNameStack[0] = strdup(packageName);

    if (packageNameStack[0] == NULL) {
        free(packageNameStack);
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    packageNameStackSize = 1;

    int ret = PPKG_OK;

    ////////////////////////////////////////////////////////////////

    while (packageNameStackSize > 0) {
        char * packageName = packageNameStack[packageNameStackSize - 1];
        packageNameStack[packageNameStackSize - 1] = NULL;
        packageNameStackSize--;

        PPKGFormula * formula = NULL;

        for (size_t i = 0; i < packageSetSize; i++) {
            if (strcmp(packageSet[i]->packageName, packageName) == 0) {
                free(packageName);

                packageName = packageSet[i]->packageName;
                formula = packageSet[i]->formula;

                size_t lastIndex = packageSetSize - 1;

                if (i != lastIndex) {
                    PPKGPackage * package = packageSet[i];

                    for (size_t j = i + 1U; j < packageSetSize; j++) {
                        packageSet[j - 1] = packageSet[j];
                    }

                    packageSet[lastIndex] = package;
                }

                break;
            }
        }

        if (formula == NULL) {
            char * formulaFilePath = NULL;

            ret = ppkg_formula_locate(packageName, &formulaFilePath);

            if (ret != PPKG_OK) {
                free(packageName);
                goto finalize;
            }

            size_t   formulaFilePath2Length = strlen(sessionDir) + strlen(packageName) + 6U;
            char     formulaFilePath2[formulaFilePath2Length];
            snprintf(formulaFilePath2, formulaFilePath2Length, "%s/%s.yml", sessionDir, packageName);

            ret = copy_file(formulaFilePath, formulaFilePath2);

            free(formulaFilePath);

            if (ret != PPKG_OK) {
                free(packageName);
                goto finalize;
            }

            ret = ppkg_formula_parse(formulaFilePath2, &formula);

            if (ret != PPKG_OK) {
                free(packageName);
                goto finalize;
            }

            if (packageSetSize == packageSetCapcity) {
                PPKGPackage ** p = (PPKGPackage**)realloc(packageSet, (packageSetCapcity + 10) * sizeof(PPKGPackage*));

                if (p == NULL) {
                    free(packageName);
                    ppkg_formula_free(formula);
                    ret = PPKG_ERROR_MEMORY_ALLOCATE;
                    goto finalize;
                }

                packageSet = p;
                packageSetCapcity += 10;
            }

            PPKGPackage * package = (PPKGPackage*)malloc(sizeof(PPKGPackage));

            if (package == NULL) {
                free(packageName);
                ppkg_formula_free(formula);
                ret = PPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            package->formula = formula;
            package->packageName = packageName;

            packageSet[packageSetSize] = package;
            packageSetSize++;

            if (formula->dep_pkg == NULL) {
                continue;
            }

            ////////////////////////////////////////////////////////////////

            size_t depPackageNamesLength = strlen(formula->dep_pkg);

            size_t depPackageNamesCopyLength = depPackageNamesLength + 1U;
            char   depPackageNamesCopy[depPackageNamesCopyLength];
            strncpy(depPackageNamesCopy, formula->dep_pkg, depPackageNamesCopyLength);

            char * depPackageName = strtok(depPackageNamesCopy, " ");

            while (depPackageName != NULL) {
                if (strcmp(depPackageName, packageName) == 0) {
                    fprintf(stderr, "package '%s' depends itself.\n", packageName);
                    ret = PPKG_ERROR;
                    goto finalize;
                }

                ////////////////////////////////////////////////////////////////

                if (packageNameStackSize == packageNameStackCapcity) {
                    char ** p = (char**)realloc(packageNameStack, (packageNameStackCapcity + 10) * sizeof(char*));

                    if (p == NULL) {
                        ret = PPKG_ERROR_MEMORY_ALLOCATE;
                        goto finalize;
                    }

                    packageNameStack = p;
                    packageNameStackCapcity += 10;
                }

                char * p = strdup(depPackageName);

                if (p == NULL) {
                    ret = PPKG_ERROR_MEMORY_ALLOCATE;
                    goto finalize;
                }

                packageNameStack[packageNameStackSize] = p;
                packageNameStackSize++;

                depPackageName = strtok (NULL, " ");
            }
        }
    }

finalize:
    for (size_t i = 0; i < packageNameStackSize; i++) {
        free(packageNameStack[i]);
        packageNameStack[i] = NULL;
    }

    free(packageNameStack);
    packageNameStack = NULL;

    if (ret == PPKG_OK) {
        (*ppackageSet) = packageSet;
        (*ppackageSetSize) = packageSetSize;
        (*ppackageSetCapcity) = packageSetCapcity;
    } else {
        for (size_t i = 0; i < packageSetSize; i++) {
            free(packageSet[i]->packageName);
            ppkg_formula_free(packageSet[i]->formula);

            packageSet[i]->formula = NULL;
            packageSet[i]->packageName = NULL;

            free(packageSet[i]);
            packageSet[i] = NULL;
        }
    }

    return ret;
}

int ppkg_install(const char * packageName, PPKGInstallOptions options) {
    const char * const userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   ppkgHomeDirLength = userHomeDirLength + 7U;
    char     ppkgHomeDir[ppkgHomeDirLength];
    snprintf(ppkgHomeDir, ppkgHomeDirLength, "%s/.ppkg", userHomeDir);

    struct stat st;

    if (stat(ppkgHomeDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", ppkgHomeDir);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(ppkgHomeDir, S_IRWXU) != 0) {
            perror(ppkgHomeDir);
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (getenv("SSL_CERT_FILE") == NULL) {
        size_t   cacertFilePathLength = ppkgHomeDirLength + 31U;
        char     cacertFilePath[cacertFilePathLength];
        snprintf(cacertFilePath, cacertFilePathLength, "%s/core/etc/ssl/certs/cacert.pem", ppkgHomeDir);

        if (stat(cacertFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            // https://www.openssl.org/docs/man1.1.1/man3/SSL_CTX_set_default_verify_paths.html
            if (setenv("SSL_CERT_FILE", cacertFilePath, 1) != 0) {
                perror("SSL_CERT_FILE");
                return PPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   ppkgInstallingDirLength = ppkgHomeDirLength + 12U;
    char     ppkgInstallingDir[ppkgInstallingDirLength];
    snprintf(ppkgInstallingDir, ppkgInstallingDirLength, "%s/installing", ppkgHomeDir);

    if (stat(ppkgInstallingDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", ppkgInstallingDir);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(ppkgInstallingDir, S_IRWXU) != 0) {
            perror(ppkgInstallingDir);
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   ppkgInstallingSessionRootDirLength = ppkgInstallingDirLength + 10U;
    char     ppkgInstallingSessionRootDir[ppkgInstallingSessionRootDirLength];
    snprintf(ppkgInstallingSessionRootDir, ppkgInstallingSessionRootDirLength, "%s/.session", ppkgInstallingDir);

    if (stat(ppkgInstallingSessionRootDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", ppkgInstallingSessionRootDir);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(ppkgInstallingSessionRootDir, S_IRWXU) != 0) {
            perror(ppkgInstallingSessionRootDir);
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   sessionDirTemplateLength = ppkgInstallingSessionRootDirLength + 16U;
    char     sessionDirTemplate[sessionDirTemplateLength];
    snprintf(sessionDirTemplate, sessionDirTemplateLength, "%s/XXXXXX", ppkgInstallingSessionRootDir);

    char *   sessionDir = mkdtemp(sessionDirTemplate);

    if (sessionDir == NULL) {
        perror(sessionDirTemplate);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t         packageSetCapcity = 0;
    size_t         packageSetSize    = 0;
    PPKGPackage ** packageSet        = NULL;

    int ret = check_and_read_formula_in_cache(packageName, sessionDir, &packageSet, &packageSetSize, &packageSetCapcity);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    SysInfo sysinfo = {0};

    char * libcName = NULL;
    char * currentExecutablePath = NULL;

    ret = sysinfo_make(&sysinfo);

    if (ret != PPKG_OK) {
        goto finalize;
    }

    switch(sysinfo.libc) {
        case 1:  libcName = (char*)"glibc"; break;
        case 2:  libcName = (char*)"musl";  break;
        default: libcName = (char*)"";
    }

    //////////////////////////////////////////////////////////////////////////////

    currentExecutablePath = self_realpath();

    if (currentExecutablePath == NULL) {
        goto finalize;
    }

    //////////////////////////////////////////////////////////////////////////////

    for (int i = packageSetSize - 1; i >= 0; i--) {
        PPKGPackage * package = packageSet[i];
        char * packageName = package->packageName;

        //printf("%d:%s\n", i, packageName);

        ret = ppkg_check_if_the_given_package_is_installed(packageName);

        if (ret == PPKG_OK) {
            fprintf(stderr, "package already has been installed : %s\n", packageName);
            continue;
        }

        char * * recursiveDependentPackageNameArrayList        = NULL;
        size_t   recursiveDependentPackageNameArrayListSize    = 0;
        size_t   recursiveDependentPackageNameArrayListCapcity = 0;

        ret = getRecursiveDependentPackageNameArrayList(packageName, packageSet, packageSetSize, &recursiveDependentPackageNameArrayList, &recursiveDependentPackageNameArrayListSize, &recursiveDependentPackageNameArrayListCapcity);

        if (ret != PPKG_OK) {
            goto finalize;
        }

        char * recursiveDependentPackageNamesString        = NULL;
        size_t recursiveDependentPackageNamesStringSize    = 0;
        size_t recursiveDependentPackageNamesStringCapcity = 0;

        for (size_t i = 1; i < recursiveDependentPackageNameArrayListSize; i++) {
            //printf("%s:: %s\n", packageName, recursiveDependentPackageNameArrayList[i]);

            char * recursiveDependentPackageName = recursiveDependentPackageNameArrayList[i];
            size_t recursiveDependentPackageNameLength = strlen(recursiveDependentPackageName);

            if (recursiveDependentPackageNameLength + recursiveDependentPackageNamesStringSize >= recursiveDependentPackageNamesStringCapcity) {
                char * p = (char*)realloc(recursiveDependentPackageNamesString, (recursiveDependentPackageNamesStringCapcity + 256) * sizeof(char));
                if (p == NULL) {
                    free(recursiveDependentPackageNameArrayList);
                    recursiveDependentPackageNameArrayList = NULL;

                    free(recursiveDependentPackageNamesString);
                    recursiveDependentPackageNamesString = NULL;

                    ret = PPKG_ERROR_MEMORY_ALLOCATE;

                    goto  finalize;
                }

                memset(p + recursiveDependentPackageNamesStringSize, 0, 256);

                recursiveDependentPackageNamesString = p;
                recursiveDependentPackageNamesStringCapcity += 256;
            }

            snprintf(recursiveDependentPackageNamesString + recursiveDependentPackageNamesStringSize, recursiveDependentPackageNameLength + 2, "%s ", recursiveDependentPackageName);
            recursiveDependentPackageNamesStringSize += recursiveDependentPackageNameLength + 1;
        }

        printf("recursiveDependentPackageNamesString.of(%s)=%s\n", packageName,  recursiveDependentPackageNamesString);
        ret = ppkg_install_package(packageName, package->formula, options, userHomeDir, userHomeDirLength, ppkgHomeDir, ppkgHomeDirLength, ppkgInstallingDir, ppkgInstallingDirLength, sessionDir, sysinfo, libcName, currentExecutablePath, st, recursiveDependentPackageNamesString);

        free(recursiveDependentPackageNameArrayList);
        recursiveDependentPackageNameArrayList = NULL;

        free(recursiveDependentPackageNamesString);
        recursiveDependentPackageNamesString = NULL;

        if (ret != PPKG_OK) {
            goto finalize;
        }
    }

finalize:
    sysinfo_free(sysinfo);
    free(currentExecutablePath);

    for (size_t i = 0; i < packageSetSize; i++) {
        free(packageSet[i]->packageName);
        ppkg_formula_free(packageSet[i]->formula);

        packageSet[i]->formula = NULL;
        packageSet[i]->packageName = NULL;

        free(packageSet[i]);
        packageSet[i] = NULL;
    }

    free(packageSet);
    packageSet = NULL;

    return ret;
}
