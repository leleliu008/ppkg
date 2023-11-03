#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "core/sysinfo.h"
#include "core/base16.h"
#include "core/http.h"
#include "core/self.h"
#include "core/exe.h"
#include "core/tar.h"
#include "core/log.h"

#include "sha256sum.h"
#include "ppkg.h"

typedef struct {
    const char * name;
    const char * value;
} KV;

typedef struct {
    const char * name;
    bool         value;
} KB;

typedef struct {
    const char * name;
    size_t       value;
} KU;


static int run_cmd(char * cmd, int output2FD) {
    fprintf(stderr, "%s==>%s %s%s%s\n", COLOR_PURPLE, COLOR_OFF, COLOR_GREEN, cmd, COLOR_OFF);

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (pid == 0) {
        if (output2FD > STDERR_FILENO) {
            if (dup2(output2FD, STDOUT_FILENO) < 0) {
                perror(NULL);
                exit(253);
            }

            if (dup2(output2FD, STDERR_FILENO) < 0) {
                perror(NULL);
                exit(254);
            }
        }

        ////////////////////////////////////////

        size_t argc = 0U;
        char*  argv[30] = {0};

        char * arg = strtok(cmd, " ");

        while (arg != NULL) {
            argv[argc] = arg;
            argc++;
            arg = strtok(NULL, " ");
        }

        ////////////////////////////////////////

        bool isPath = false;

        const char * p = argv[0];

        for (;;) {
            if (p[0] == '\0') {
                break;
            }

            if (p[0] == '/') {
                isPath = true;
                break;
            }

            p++;
        }

        ////////////////////////////////////////

        if (isPath) {
            execv (argv[0], argv);
        } else {
            execvp(argv[0], argv);
        }

        perror(argv[0]);
        exit(255);
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

static int get_the_first_n_bytes_of_a_file(const char * fp, unsigned int n, char buf[]) {
    int fd = open(fp, O_RDONLY);

    if (fd == -1) {
        perror(fp);
        return PPKG_ERROR;
    }

    ssize_t readSize = read(fd, buf, n);

    if (readSize == -1) {
        perror(fp);
        close(fd);
        return PPKG_ERROR;
    } else {
        close(fd);
        return PPKG_OK;
    }
}

static int write_to_file(const char * fp, const char * str) {
    int fd = open(fp, O_WRONLY | O_CREAT | O_TRUNC, 0666);

    if (fd == -1) {
        perror(fp);
        return PPKG_ERROR;
    }

    size_t  n = strlen(str);

    ssize_t writtenSize = write(fd, str, n);

    if (writtenSize == -1) {
        perror(fp);
        close(fd);
        return PPKG_ERROR;
    }

    if (writtenSize == (ssize_t)n) {
        return PPKG_OK;
    } else {
        fprintf(stderr, "file not fully written: %s\n", fp);
        return PPKG_ERROR;
    }
}

//static void export_p() {
    //for (int i = 0; ; i++) {
    //    const char * p = __environ[i];

    //    if (p == NULL) {
    //        break;
    //    }

    //    puts(p);
    //}
//}

static int download_via_http(const char * url, const char * uri, const char * expectedSHA256SUM, const char * downloadDIR, size_t downloadDIRLength, const char * unpackDIR, size_t unpackDIRLength, bool verbose) {
    char fileNameExtension[21] = {0};

    int ret = ppkg_examine_file_extension_from_url(url, fileNameExtension, 20);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t fileNameCapacity = strlen(expectedSHA256SUM) + strlen(fileNameExtension) + 1U;
    char   fileName[fileNameCapacity];

    ret = snprintf(fileName, fileNameCapacity, "%s%s", expectedSHA256SUM, fileNameExtension);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    size_t filePathCapacity = downloadDIRLength + fileNameCapacity + 1U;
    char   filePath[filePathCapacity];

    ret = snprintf(filePath, filePathCapacity, "%s/%s", downloadDIR, fileName);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    bool needFetch = true;

    struct stat st;

    if (stat(filePath, &st) == 0 && S_ISREG(st.st_mode)) {
        char actualSHA256SUM[65] = {0};

        ret = sha256sum_of_file(actualSHA256SUM, filePath);

        if (ret != 0) {
            return ret;
        }

        if (strcmp(actualSHA256SUM, expectedSHA256SUM) == 0) {
            needFetch = false;

            if (verbose) {
                fprintf(stderr, "%s already have been fetched.\n", filePath);
            }
        }
    }

    if (needFetch) {
        size_t tmpStrLength = strlen(url) + 30U;
        char   tmpStr[tmpStrLength];

        ret = snprintf(tmpStr, tmpStrLength, "%s|%ld|%d", url, time(NULL), getpid());

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        char tmpFileName[65] = {0};

        ret = sha256sum_of_string(tmpFileName, tmpStr);

        if (ret != 0) {
            return PPKG_ERROR;
        }

        size_t tmpFilePathLength = downloadDIRLength + 65U;
        char   tmpFilePath[tmpFilePathLength];

        ret = snprintf(tmpFilePath, tmpFilePathLength, "%s/%s", downloadDIR, tmpFileName);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        ret = ppkg_http_fetch_to_file(url, tmpFilePath, verbose, verbose);

        if (ret != PPKG_OK) {
            if (uri != NULL) {
                ret = ppkg_http_fetch_to_file(uri, tmpFilePath, verbose, verbose);
            }
        }

        if (ret != PPKG_OK) {
            return ret;
        }

        char actualSHA256SUM[65] = {0};

        ret = sha256sum_of_file(actualSHA256SUM, tmpFilePath);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (strcmp(actualSHA256SUM, expectedSHA256SUM) == 0) {
            if (rename(tmpFilePath, filePath) == 0) {
                printf("%s\n", filePath);
            } else {
                perror(filePath);
                return PPKG_ERROR;
            }
        } else {
            fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", expectedSHA256SUM, actualSHA256SUM);
            return PPKG_ERROR_SHA256_MISMATCH;
        }
    }

    if (strcmp(fileNameExtension, ".zip") == 0 ||
        strcmp(fileNameExtension, ".tgz") == 0 ||
        strcmp(fileNameExtension, ".txz") == 0 ||
        strcmp(fileNameExtension, ".tlz") == 0 ||
        strcmp(fileNameExtension, ".tbz2") == 0) {

        ret = tar_extract(unpackDIR, filePath, ARCHIVE_EXTRACT_TIME, verbose, 1);

        if (ret != 0) {
            return abs(ret) + PPKG_ERROR_ARCHIVE_BASE;
        }
    } else {
        size_t toFilePathLength = unpackDIRLength + fileNameCapacity + 1U;
        char   toFilePath[toFilePathLength];

        ret = snprintf(toFilePath, toFilePathLength, "%s/%s", unpackDIR, fileName);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        ret = ppkg_copy_file(filePath, toFilePath);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    return PPKG_OK;
}

static int setup_rust_toolchain(const PPKGInstallOptions installOptions, const char * sessionDIR, const size_t sessionDIRLength) {
    const char * cargoHomeDIR = getenv("CARGO_HOME");

    if (cargoHomeDIR == NULL || cargoHomeDIR[0] == '\0') {
        const char * const userHomeDIR = getenv("HOME");

        if (userHomeDIR == NULL || userHomeDIR[0] == '\0') {
            return PPKG_ERROR_ENV_HOME_NOT_SET;
        }

        size_t   defaultCargoHomeDIRCapacity = strlen(userHomeDIR) + 8U;
        char     defaultCargoHomeDIR[defaultCargoHomeDIRCapacity];
        snprintf(defaultCargoHomeDIR, defaultCargoHomeDIRCapacity, "%s/.cargo", userHomeDIR);

        if (setenv("CARGO_HOME", defaultCargoHomeDIR, 1) != 0) {
            perror("CARGO_HOME");
            return PPKG_ERROR;
        }

        cargoHomeDIR = getenv("CARGO_HOME");
    }

    size_t cargoHomeDIRLength = strlen(cargoHomeDIR);

    size_t   rustupCommandPathCapacity = cargoHomeDIRLength + 12U;
    char     rustupCommandPath[rustupCommandPathCapacity];
    snprintf(rustupCommandPath, rustupCommandPathCapacity, "%s/bin/rustup", cargoHomeDIR);

    size_t   cargoCommandPathCapacity = cargoHomeDIRLength + 11U;
    char     cargoCommandPath[cargoCommandPathCapacity];
    snprintf(cargoCommandPath, cargoCommandPathCapacity, "%s/bin/cargo", cargoHomeDIR);

    struct stat st;

    bool  cargoExist = (stat( cargoCommandPath, &st) == 0) && S_ISREG(st.st_mode);
    bool rustupExist = (stat(rustupCommandPath, &st) == 0) && S_ISREG(st.st_mode);

    if (!(cargoExist && rustupExist)) {
        LOG_INFO("rustup and cargo commands are required, but they are not found on this machine, ppkg will install them via running shell script.");

        size_t rustupInitScriptFilePathCapacity = sessionDIRLength + 16U;
        char   rustupInitScriptFilePath[rustupInitScriptFilePathCapacity];

        int ret = snprintf(rustupInitScriptFilePath, rustupInitScriptFilePathCapacity, "%s/rustup-init.sh", sessionDIR);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        ret = ppkg_http_fetch_to_file("https://sh.rustup.rs", rustupInitScriptFilePath, installOptions.verbose_net, installOptions.verbose_net);

        if (ret != PPKG_OK) {
            return ret;
        }

        size_t cmdCapacity = rustupInitScriptFilePathCapacity + 10U;
        char   cmd[cmdCapacity];

        ret = snprintf(cmd, cmdCapacity, "bash %s -y", rustupInitScriptFilePath);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        ret = run_cmd(cmd, STDOUT_FILENO);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    const char * const PATH = getenv("PATH");

    if (PATH == NULL || PATH[0] == '\0') {
        return PPKG_ERROR_ENV_PATH_NOT_SET;
    }

    size_t   newPATHLength = cargoHomeDIRLength + strlen(PATH) + 5U;
    char     newPATH[newPATHLength];
    snprintf(newPATH, newPATHLength, "%s/bin:%s", cargoHomeDIR, PATH);

    if (setenv("PATH", newPATH, 1) != 0) {
        perror("PATH");
        return PPKG_ERROR;
    }

    return PPKG_OK;
}


#define BUILD_SYSTEM_TYPE_CMAKE 1
#define BUILD_SYSTEM_TYPE_CONFIGURE 2

#define NATIVE_PACKAGE_ID_ZLIB     1
#define NATIVE_PACKAGE_ID_LIBBZ2   2
#define NATIVE_PACKAGE_ID_LIBLZMA  3

#define NATIVE_PACKAGE_ID_PERL     4
#define NATIVE_PACKAGE_ID_OPENSSL  5
#define NATIVE_PACKAGE_ID_TEXINFO  6
#define NATIVE_PACKAGE_ID_HELP2MAN 7
#define NATIVE_PACKAGE_ID_LIBTOOL  8
#define NATIVE_PACKAGE_ID_AUTOCONF 9
#define NATIVE_PACKAGE_ID_AUTOMAKE 10

#define NATIVE_PACKAGE_ID_INTLTOOL 11
#define NATIVE_PACKAGE_ID_EXPAT    12
#define NATIVE_PACKAGE_ID_GDBM     13
#define NATIVE_PACKAGE_ID_SQLITE3  14
#define NATIVE_PACKAGE_ID_LIBFFI   15
#define NATIVE_PACKAGE_ID_LIBYAML  16
#define NATIVE_PACKAGE_ID_PYTHON3  17
#define NATIVE_PACKAGE_ID_RUBY     18
#define NATIVE_PACKAGE_ID_PERL_XML_PARSER 19

typedef struct {
    const char * name;

    const char * srcUrl;
    const char * srcSha;

    const char * buildConfigureArgs;

    int buildSystemType;

    int depPackageIDArray[10];
} NativePackage;

static int getNativePackageInfoByID(int packageID, NativePackage * nativePackage) {
    switch (packageID) {
        case NATIVE_PACKAGE_ID_EXPAT:
            nativePackage->name = "expat";
            nativePackage->srcUrl = "https://github.com/libexpat/libexpat/releases/download/R_2_5_0/expat-2.5.0.tar.xz";
            nativePackage->srcSha = "ef2420f0232c087801abf705e89ae65f6257df6b7931d37846a193ef2e8cdcbe";
            nativePackage->buildConfigureArgs = "-DEXPAT_BUILD_DOCS=OFF -DEXPAT_BUILD_TESTS=OFF -DEXPAT_BUILD_FUZZERS=OFF -DEXPAT_BUILD_EXAMPLES=OFF -DEXPAT_BUILD_TOOLS=OFF";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CMAKE;
            break;
        case NATIVE_PACKAGE_ID_ZLIB:
            nativePackage->name = "zlib";
            nativePackage->srcUrl = "https://zlib.net/fossils/zlib-1.3.tar.gz";
            nativePackage->srcSha = "ff0ba4c292013dbc27530b3a81e1f9a813cd39de01ca5e0f8bf355702efa593e";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CMAKE;
            break;
        case NATIVE_PACKAGE_ID_LIBBZ2:
            nativePackage->name = "libbz2";
            nativePackage->srcUrl = "https://github.com/leleliu008/bzip2/archive/refs/tags/1.0.8.tar.gz";
            nativePackage->srcSha = "fb36d769189faaf841390fae88639fb02c79b87b0691a340fbbfc32b4f82b789";
            nativePackage->buildConfigureArgs = "-DINSTALL_EXECUTABLES=OFF -DINSTALL_LIBRARIES=ON -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CMAKE;
            break;
        case NATIVE_PACKAGE_ID_PERL:
            nativePackage->name = "perl";
            nativePackage->srcUrl = "https://cpan.metacpan.org/authors/id/R/RJ/RJBS/perl-5.36.0.tar.xz";
            nativePackage->srcSha = "0f386dccbee8e26286404b2cca144e1005be65477979beb9b1ba272d4819bcf0";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_OPENSSL:
            nativePackage->name = "openssl";
            nativePackage->srcUrl = "https://www.openssl.org/source/openssl-3.1.1.tar.gz";
            nativePackage->srcSha = "b3aa61334233b852b63ddb048df181177c2c659eb9d4376008118f9c08d07674";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_PERL;
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_TEXINFO:
            nativePackage->name = "texinfo";
            nativePackage->srcUrl = "https://ftp.gnu.org/gnu/texinfo/texinfo-7.1.tar.xz";
            nativePackage->srcSha = "deeec9f19f159e046fdf8ad22231981806dac332cc372f1c763504ad82b30953";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_PERL;
            nativePackage->buildConfigureArgs = "--with-included-regex --enable-threads=posix --disable-nls";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_HELP2MAN:
            nativePackage->name = "help2man";
            nativePackage->srcUrl = "https://ftp.gnu.org/gnu/help2man/help2man-1.49.3.tar.xz";
            nativePackage->srcSha = "4d7e4fdef2eca6afe07a2682151cea78781e0a4e8f9622142d9f70c083a2fd4f";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_PERL;
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_INTLTOOL:
            nativePackage->name = "intltool";
            nativePackage->srcUrl = "https://launchpad.net/intltool/trunk/0.51.0/+download/intltool-0.51.0.tar.gz";
            nativePackage->srcSha = "67c74d94196b153b774ab9f89b2fa6c6ba79352407037c8c14d5aeb334e959cd";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_PERL_XML_PARSER;
            nativePackage->buildConfigureArgs = "";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_LIBTOOL:
            nativePackage->name = "libtool";
            nativePackage->srcUrl = "https://ftp.gnu.org/gnu/libtool/libtool-2.4.7.tar.xz";
            nativePackage->srcSha = "4f7f217f057ce655ff22559ad221a0fd8ef84ad1fc5fcb6990cecc333aa1635d";
            nativePackage->buildConfigureArgs = "--enable-ltdl-install";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_AUTOCONF:
            nativePackage->name = "autoconf";
            nativePackage->srcUrl = "https://ftp.gnu.org/gnu/autoconf/autoconf-2.71.tar.gz";
            nativePackage->srcSha = "431075ad0bf529ef13cb41e9042c542381103e80015686222b8a9d4abef42a1c";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_PERL;
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_AUTOMAKE:
            nativePackage->name = "automake";
            nativePackage->srcUrl = "https://ftp.gnu.org/gnu/automake/automake-1.16.5.tar.xz";
            nativePackage->srcSha = "f01d58cd6d9d77fbdca9eb4bbd5ead1988228fdb73d6f7a201f5f8d6b118b469";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_AUTOCONF;
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_LIBLZMA:
            nativePackage->name = "liblzma";
            nativePackage->srcUrl = "https://github.com/tukaani-project/xz/releases/download/v5.4.4/xz-5.4.4.tar.gz";
            nativePackage->srcSha = "aae39544e254cfd27e942d35a048d592959bd7a79f9a624afb0498bb5613bdf8";
            nativePackage->buildConfigureArgs = "--disable-dependency-tracking --enable-static --disable-shared --disable-nls --enable-largefile --disable-xz --disable-xzdec --disable-lzmadec --disable-lzmainfo --disable-lzma-links --disable-scripts --disable-doc";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
            ;;
        case NATIVE_PACKAGE_ID_GDBM:
            nativePackage->name = "gdbm";
            nativePackage->srcUrl = "https://ftp.gnu.org/gnu/gdbm/gdbm-1.23.tar.gz";
            nativePackage->srcSha = "74b1081d21fff13ae4bd7c16e5d6e504a4c26f7cde1dca0d963a484174bbcacd";
            nativePackage->buildConfigureArgs = "--disable-dependency-tracking --enable-static --disable-shared --disable-nls --enable-largefile --enable-libgdbm-compat --without-readline";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_SQLITE3:
            nativePackage->name = "sqlite3";
            nativePackage->srcUrl = "https://www.sqlite.org/2023/sqlite-autoconf-3410100.tar.gz";
            nativePackage->srcSha = "4dadfbeab9f8e16c695d4fbbc51c16b2f77fb97ff4c1c3d139919dfc038c9e33";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_ZLIB;
            nativePackage->buildConfigureArgs = "--disable-dependency-tracking --enable-static --disable-shared --enable-largefile --disable-editline --disable-readline";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_LIBFFI:
            nativePackage->name = "libffi";
            nativePackage->srcUrl = "https://github.com/libffi/libffi/releases/download/v3.4.4/libffi-3.4.4.tar.gz";
            nativePackage->srcSha = "d66c56ad259a82cf2a9dfc408b32bf5da52371500b84745f7fb8b645712df676";
            nativePackage->buildConfigureArgs = "--disable-dependency-tracking --enable-static --disable-shared --disable-docs --disable-symvers";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_LIBYAML:
            nativePackage->name = "libyaml";
            nativePackage->srcUrl = "https://github.com/yaml/libyaml/releases/download/0.2.5/yaml-0.2.5.tar.gz";
            nativePackage->srcSha = "c642ae9b75fee120b2d96c712538bd2cf283228d2337df2cf2988e3c02678ef4";
            nativePackage->buildConfigureArgs = "--disable-dependency-tracking --enable-static --disable-shared --enable-largefile";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_RUBY:
            nativePackage->name = "ruby";
            nativePackage->srcUrl = "https://cache.ruby-lang.org/pub/ruby/3.2/ruby-3.2.2.tar.gz";
            nativePackage->srcSha = "96c57558871a6748de5bc9f274e93f4b5aad06cd8f37befa0e8d94e7b8a423bc";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_ZLIB;
            nativePackage->depPackageIDArray[1] = NATIVE_PACKAGE_ID_LIBFFI;
            nativePackage->depPackageIDArray[2] = NATIVE_PACKAGE_ID_LIBYAML;
            nativePackage->buildConfigureArgs = "--disable-dependency-tracking --enable-static --enable-shared --disable-docs";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_PYTHON3:
            nativePackage->name = "python3";
            nativePackage->srcUrl = "https://www.python.org/ftp/python/3.11.5/Python-3.11.5.tgz";
            nativePackage->srcSha = "a12a0a013a30b846c786c010f2c19dd36b7298d888f7c4bd1581d90ce18b5e58";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_ZLIB;
            nativePackage->depPackageIDArray[1] = NATIVE_PACKAGE_ID_LIBBZ2;
            nativePackage->depPackageIDArray[2] = NATIVE_PACKAGE_ID_LIBLZMA;
            nativePackage->depPackageIDArray[3] = NATIVE_PACKAGE_ID_LIBFFI;
            nativePackage->depPackageIDArray[4] = NATIVE_PACKAGE_ID_GDBM;
            nativePackage->depPackageIDArray[5] = NATIVE_PACKAGE_ID_EXPAT;
            nativePackage->depPackageIDArray[6] = NATIVE_PACKAGE_ID_SQLITE3;
            nativePackage->depPackageIDArray[7] = NATIVE_PACKAGE_ID_PERL;
            nativePackage->depPackageIDArray[8] = NATIVE_PACKAGE_ID_OPENSSL;
            nativePackage->buildConfigureArgs = "--with-system-expat --with-system-ffi --with-ensurepip=yes --with-lto --enable-ipv6 --enable-shared --enable-largefile --disable-option-checking --disable-nls --disable-debug --disable-loadable-sqlite-extensions --disable-profiling";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_PERL_XML_PARSER:
            nativePackage->name = "perl-XML-Parser";
            nativePackage->srcUrl = "https://cpan.metacpan.org/authors/id/T/TO/TODDR/XML-Parser-2.46.tar.gz";
            nativePackage->srcSha = "d331332491c51cccfb4cb94ffc44f9cd73378e618498d4a37df9e043661c515d";
            nativePackage->depPackageIDArray[1] = NATIVE_PACKAGE_ID_PERL;
            nativePackage->depPackageIDArray[2] = NATIVE_PACKAGE_ID_EXPAT;
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        default:
            fprintf(stderr, "unknown native package id: %d\n", packageID);
            return PPKG_ERROR;
    }

    return PPKG_OK;
}

static int export_environment_variables_for_build_tools(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    struct stat st;

    size_t includeDIRLength = packageInstalledDIRCapacity + 9U;
    char   includeDIR[includeDIRLength];

    int ret = snprintf(includeDIR, includeDIRLength, "%s/include", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (stat(includeDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
        const char * const CPPFLAGS = getenv("CPPFLAGS");

        if (CPPFLAGS == NULL || CPPFLAGS[0] == '\0') {
            size_t newCPPFLAGSLength = includeDIRLength + 3U;
            char   newCPPFLAGS[newCPPFLAGSLength];

            ret = snprintf(newCPPFLAGS, newCPPFLAGSLength, "-I%s", includeDIR);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            if (setenv("CPPFLAGS", newCPPFLAGS, 1) != 0) {
                perror("CPPFLAGS");
                return PPKG_ERROR;
            }
        } else {
            size_t newCPPFLAGSLength = includeDIRLength + strlen(CPPFLAGS) + 4U;
            char   newCPPFLAGS[newCPPFLAGSLength];

            ret = snprintf(newCPPFLAGS, newCPPFLAGSLength, "-I%s %s", includeDIR, CPPFLAGS);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            if (setenv("CPPFLAGS", newCPPFLAGS, 1) != 0) {
                perror("CPPFLAGS");
                return PPKG_ERROR;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t libDIRLength = packageInstalledDIRCapacity + 5U;
    char   libDIR[libDIRLength];

    ret = snprintf(libDIR, libDIRLength, "%s/lib", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (stat(libDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
        const char * const LDFLAGS = getenv("LDFLAGS");

        if (LDFLAGS == NULL || LDFLAGS[0] == '\0') {
            size_t newLDFLAGSLength = libDIRLength + 3U;
            char   newLDFLAGS[newLDFLAGSLength];

            ret = snprintf(newLDFLAGS, newLDFLAGSLength, "-L%s", libDIR);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            if (setenv("LDFLAGS", newLDFLAGS, 1) != 0) {
                perror("LDFLAGS");
                return PPKG_ERROR;
            }
        } else {
            size_t newLDFLAGSLength = (libDIRLength << 1U) + strlen(LDFLAGS) + 15U;
            char   newLDFLAGS[newLDFLAGSLength];

            ret = snprintf(newLDFLAGS, newLDFLAGSLength, "%s -L%s -Wl,-rpath,%s", LDFLAGS, libDIR, libDIR);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            if (setenv("LDFLAGS", newLDFLAGS, 1) != 0) {
                perror("LDFLAGS");
                return PPKG_ERROR;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    const char * a[2] = { "lib", "share" };

    for (int i = 0; i < 2; i++) {
        size_t pkgconfigDIRLength = packageInstalledDIRCapacity + 20U;
        char   pkgconfigDIR[pkgconfigDIRLength];

        ret = snprintf(pkgconfigDIR, pkgconfigDIRLength, "%s/%s/pkgconfig", packageInstalledDIR, a[i]);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (stat(pkgconfigDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
            const char * const PKG_CONFIG_PATH = getenv("PKG_CONFIG_PATH");

            if (PKG_CONFIG_PATH == NULL || PKG_CONFIG_PATH[0] == '\0') {
                if (setenv("PKG_CONFIG_PATH", pkgconfigDIR, 1) != 0) {
                    perror("PKG_CONFIG_PATH");
                    return PPKG_ERROR;
                }
            } else {
                size_t newPKG_CONFIG_PATHLength = pkgconfigDIRLength + strlen(PKG_CONFIG_PATH) + 2U;
                char   newPKG_CONFIG_PATH[newPKG_CONFIG_PATHLength];

                ret = snprintf(newPKG_CONFIG_PATH, newPKG_CONFIG_PATHLength, "%s:%s", pkgconfigDIR, PKG_CONFIG_PATH);

                if (ret < 0) {
                    perror(NULL);
                    return PPKG_ERROR;
                }

                if (setenv("PKG_CONFIG_PATH", newPKG_CONFIG_PATH, 1) != 0) {
                    perror("PKG_CONFIG_PATH");
                    return PPKG_ERROR;
                }
            }
        }
    }

    return PPKG_OK;
}

static int export_environment_variables_for_other_tools(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    struct stat st;

    size_t binDIRLength = packageInstalledDIRCapacity + 5U;
    char   binDIR[binDIRLength];

    int ret = snprintf(binDIR, binDIRLength, "%s/bin", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    size_t sbinDIRLength = packageInstalledDIRCapacity + 6U;
    char   sbinDIR[sbinDIRLength];

    ret = snprintf(sbinDIR, sbinDIRLength, "%s/sbin", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    bool  binDIRExists = stat( binDIR, &st) == 0 && S_ISDIR(st.st_mode);
    bool sbinDIRExists = stat(sbinDIR, &st) == 0 && S_ISDIR(st.st_mode);

    if (binDIRExists || sbinDIRExists) {
        const char * const PATH = getenv("PATH");

        if (PATH == NULL || PATH[0] == '\0') {
            return PPKG_ERROR_ENV_PATH_NOT_SET;
        }

        if (binDIRExists && sbinDIRExists) {
            size_t newPATHLength = binDIRLength + sbinDIRLength + strlen(PATH) + 3U;
            char   newPATH[newPATHLength];

            ret = snprintf(newPATH, newPATHLength, "%s:%s:%s", binDIR, sbinDIR, PATH);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            if (setenv("PATH", newPATH, 1) != 0) {
                perror("PATH");
                return PPKG_ERROR;
            }
        } else if (binDIRExists) {
            size_t newPATHLength = binDIRLength + strlen(PATH) + 2U;
            char   newPATH[newPATHLength];

            ret = snprintf(newPATH, newPATHLength, "%s:%s", binDIR, PATH);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            if (setenv("PATH", newPATH, 1) != 0) {
                perror("PATH");
                return PPKG_ERROR;
            }
        } else if (sbinDIRExists) {
            size_t newPATHLength = sbinDIRLength + strlen(PATH) + 2U;
            char   newPATH[newPATHLength];

            ret = snprintf(newPATH, newPATHLength, "%s:%s", sbinDIR, PATH);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            if (setenv("PATH", newPATH, 1) != 0) {
                perror("PATH");
                return PPKG_ERROR;
            }
        }
    }

    size_t shareDIRLength = packageInstalledDIRCapacity + 7U;
    char   shareDIR[shareDIRLength];

    ret = snprintf(shareDIR, shareDIRLength, "%s/share", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    // https://www.gnu.org/software/automake/manual/html_node/Macro-Search-Path.html

    size_t aclocalDIRLength = shareDIRLength + 9U;
    char   aclocalDIR[aclocalDIRLength];

    ret = snprintf(aclocalDIR, aclocalDIRLength, "%s/aclocal", shareDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (stat(aclocalDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
        const char * const ACLOCAL_PATH = getenv("ACLOCAL_PATH");

        if (ACLOCAL_PATH == NULL || ACLOCAL_PATH[0] == '\0') {
            if (setenv("ACLOCAL_PATH", aclocalDIR, 1) != 0) {
                perror("ACLOCAL_PATH");
                return PPKG_ERROR;
            }
        } else {
            size_t newACLOCAL_PATHLength = aclocalDIRLength + strlen(ACLOCAL_PATH) + 2U;
            char   newACLOCAL_PATH[newACLOCAL_PATHLength];

            ret = snprintf(newACLOCAL_PATH, newACLOCAL_PATHLength, "%s:%s", aclocalDIR, ACLOCAL_PATH);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            if (setenv("ACLOCAL_PATH", newACLOCAL_PATH, 1) != 0) {
                perror("ACLOCAL_PATH");
                return PPKG_ERROR;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    // https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
    // https://gi.readthedocs.io/en/latest/tools/g-ir-scanner.html#environment-variables
    // https://help.gnome.org/admin//system-admin-guide/2.32/mimetypes-database.html.en

    size_t girSearchDIRLength = shareDIRLength + 9U;
    char   girSearchDIR[girSearchDIRLength];

    ret = snprintf(girSearchDIR, girSearchDIRLength, "%s/gir-1.0", shareDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    size_t mimeSearchDIRLength = shareDIRLength + 6U;
    char   mimeSearchDIR[mimeSearchDIRLength];

    ret = snprintf(mimeSearchDIR, mimeSearchDIRLength, "%s/mime", shareDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if ((stat(girSearchDIR, &st) == 0 && S_ISDIR(st.st_mode)) || (stat(mimeSearchDIR, &st) == 0 && S_ISDIR(st.st_mode))) {
        const char * const XDG_DATA_DIRS = getenv("XDG_DATA_DIRS");

        if (XDG_DATA_DIRS == NULL || XDG_DATA_DIRS[0] == '\0') {
            if (setenv("XDG_DATA_DIRS", shareDIR, 1) != 0) {
                perror("XDG_DATA_DIRS");
                return PPKG_ERROR;
            }
        } else {
            size_t newXDG_DATA_DIRSLength = shareDIRLength + strlen(XDG_DATA_DIRS) + 2U;
            char   newXDG_DATA_DIRS[newXDG_DATA_DIRSLength];

            ret = snprintf(newXDG_DATA_DIRS, newXDG_DATA_DIRSLength, "%s:%s", shareDIR, XDG_DATA_DIRS);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            if (setenv("XDG_DATA_DIRS", newXDG_DATA_DIRS, 1) != 0) {
                perror("XDG_DATA_DIRS");
                return PPKG_ERROR;
            }
        }
    }

    return PPKG_OK;
}

typedef struct {
    const char * name;
    const char * libs;
    const char * env1;
    const char * env2;
} XX;

static int install_native_package(
        int nativePackageID,
        const char * downloadsDIR,
        const size_t downloadsDIRLength,
        const char * nativePackageInstallingRootDIR,
        const size_t nativePackageInstallingRootDIRLength,
        const char * nativePackageInstalledRootDIR,
        const size_t nativePackageInstalledRootDIRLength,
        const size_t njobs,
        const PPKGInstallOptions installOptions) {
    NativePackage nativePackage = {0};

    int ret = getNativePackageInfoByID(nativePackageID, &nativePackage);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < 10; i++) {
        if (nativePackage.depPackageIDArray[i] == 0) {
            break;
        }

        ret = install_native_package(nativePackage.depPackageIDArray[i], downloadsDIR, downloadsDIRLength, nativePackageInstallingRootDIR, nativePackageInstallingRootDIRLength, nativePackageInstalledRootDIR, nativePackageInstalledRootDIRLength, njobs, installOptions);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * packageName = nativePackage.name;
    const char * srcUrl = nativePackage.srcUrl;
    const char * srcSha = nativePackage.srcSha;
    const char * buildConfigureArgs = nativePackage.buildConfigureArgs;
    int          buildSystemType = nativePackage.buildSystemType;

    if (buildConfigureArgs == NULL) {
        buildConfigureArgs = "";
    }

    printf("install native package : %s\n", packageName);

    //////////////////////////////////////////////////////////////////////////////

    size_t   packageNameLength = strlen(packageName);

    size_t   receiptFilePathLength = nativePackageInstalledRootDIRLength + packageNameLength + 14U;
    char     receiptFilePath[receiptFilePathLength];
    snprintf(receiptFilePath, receiptFilePathLength, "%s/%s/receipt.txt", nativePackageInstalledRootDIR, packageName);

    struct stat st;

    if (stat(receiptFilePath, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            char buf[65] = {0};

            ret = get_the_first_n_bytes_of_a_file(receiptFilePath, 64, buf);

            if (ret != PPKG_OK) {
                return ret;
            }

            if (strcmp(buf, srcSha) == 0) {
                fprintf(stderr, "native package '%s' already has been installed.\n", packageName);

                size_t   packageInstalledDIRCapacity = nativePackageInstalledRootDIRLength + packageNameLength + 2U;
                char     packageInstalledDIR[packageInstalledDIRCapacity];
                snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", nativePackageInstalledRootDIR, packageName);

                ret = export_environment_variables_for_build_tools(packageInstalledDIR, packageInstalledDIRCapacity);

                if (ret != PPKG_OK) {
                    return ret;
                }

                return export_environment_variables_for_other_tools(packageInstalledDIR, packageInstalledDIRCapacity);
            }
        } else {
            fprintf(stderr, "%s was expected to be a regular file, but it was not.\n", receiptFilePath);
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   nativePackageWorkingTopDIRLength = nativePackageInstallingRootDIRLength + packageNameLength + 2U;
    char     nativePackageWorkingTopDIR[nativePackageWorkingTopDIRLength];
    snprintf(nativePackageWorkingTopDIR, nativePackageWorkingTopDIRLength, "%s/%s", nativePackageInstallingRootDIR, packageName);

    size_t   nativePackageWorkingSrcDIRLength = nativePackageWorkingTopDIRLength + 5U;
    char     nativePackageWorkingSrcDIR[nativePackageWorkingSrcDIRLength];
    snprintf(nativePackageWorkingSrcDIR, nativePackageWorkingSrcDIRLength, "%s/src", nativePackageWorkingTopDIR);

    ret = download_via_http(srcUrl, NULL, srcSha, downloadsDIR, downloadsDIRLength, nativePackageWorkingSrcDIR, nativePackageWorkingSrcDIRLength, installOptions.verbose_net);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (chdir(nativePackageWorkingSrcDIR) != 0) {
        perror(nativePackageWorkingSrcDIR);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   strBufSize = packageNameLength + strlen(srcUrl) + strlen(srcSha) + 50U;
    char     strBuf[strBufSize];
    snprintf(strBuf, strBufSize, "%s:%s:%s:%zu:%u", packageName, srcUrl, srcSha, time(NULL), getpid());

    char packageInstalledSHA[65] = {0};

    ret = sha256sum_of_string(packageInstalledSHA, strBuf);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t   packageInstalledDIRCapacity = nativePackageInstalledRootDIRLength + 66U;
    char     packageInstalledDIR[packageInstalledDIRCapacity];
    snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", nativePackageInstalledRootDIR, packageInstalledSHA);

    size_t   nativePackageWorkingLibDIRLength = nativePackageWorkingTopDIRLength + 5U;
    char     nativePackageWorkingLibDIR[nativePackageWorkingLibDIRLength];
    snprintf(nativePackageWorkingLibDIR, nativePackageWorkingLibDIRLength, "%s/lib", nativePackageWorkingTopDIR);

    size_t   nativePackageWorkingIncludeDIRLength = nativePackageWorkingTopDIRLength + 9U;
    char     nativePackageWorkingIncludeDIR[nativePackageWorkingIncludeDIRLength];
    snprintf(nativePackageWorkingIncludeDIR, nativePackageWorkingIncludeDIRLength, "%s/include", nativePackageWorkingTopDIR);

    size_t   nativePackageWorkingPkgconfigDIRLength = nativePackageWorkingTopDIRLength + 15U;
    char     nativePackageWorkingPkgconfigDIR[nativePackageWorkingPkgconfigDIRLength];
    snprintf(nativePackageWorkingPkgconfigDIR, nativePackageWorkingPkgconfigDIRLength, "%s/lib/pkgconfig", nativePackageWorkingTopDIR);

    // override the default search directory (usually /usr/lib/pkgconfig:/usr/share/pkgconfig)
    // because we only want to use our own
    if (setenv("PKG_CONFIG_LIBDIR", nativePackageWorkingLibDIR, 1) != 0) {
        perror("PKG_CONFIG_LIBDIR");
        return PPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t   packageInstalledLibraryDIRLength = packageInstalledDIRCapacity + 5U;
    char     packageInstalledLibraryDIR[packageInstalledLibraryDIRLength];
    snprintf(packageInstalledLibraryDIR, packageInstalledLibraryDIRLength, "%s/lib", packageInstalledDIR);

    const char * const LDFLAGS = getenv("LDFLAGS");

    if (LDFLAGS == NULL || LDFLAGS[0] == '\0') {
        size_t   newLDFLAGSLength = packageInstalledLibraryDIRLength + 12U;
        char     newLDFLAGS[newLDFLAGSLength];
        snprintf(newLDFLAGS, newLDFLAGSLength, "-Wl,-rpath,%s", packageInstalledLibraryDIR);

        if (setenv("LDFLAGS", newLDFLAGS, 1) != 0) {
            perror("LDFLAGS");
            return PPKG_ERROR;
        }
    } else {
        size_t   newLDFLAGSLength = packageInstalledLibraryDIRLength + strlen(LDFLAGS) + 15U;
        char     newLDFLAGS[newLDFLAGSLength];
        snprintf(newLDFLAGS, newLDFLAGSLength, "-Wl,-rpath,%s %s", packageInstalledLibraryDIR, LDFLAGS);

        if (setenv("LDFLAGS", newLDFLAGS, 1) != 0) {
            perror("LDFLAGS");
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (nativePackageID == NATIVE_PACKAGE_ID_TEXINFO) {
        const char * cmd = "gsed -i /libintl/d tp/Texinfo/XS/parsetexi/api.c";

        const size_t cmdLength = strlen(cmd);

        char cmdCopy[cmdLength + 1U];

        strncpy(cmdCopy, cmd, cmdLength);

        cmdCopy[cmdLength] = '\0';

        ret = run_cmd(cmdCopy, STDOUT_FILENO);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (buildSystemType == BUILD_SYSTEM_TYPE_CMAKE) {
        char   cmakePath[PATH_MAX];
        size_t cmakePathLength = 0U;

        ret = exe_where("cmake", cmakePath, PATH_MAX, &cmakePathLength);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (cmakePathLength == 0U) {
            fprintf(stderr, "cmake command was not found.\n");
            return PPKG_ERROR;
        }

        size_t configurePhaseCmdLength = cmakePathLength + packageInstalledDIRCapacity + strlen(buildConfigureArgs) + 124U;
        char   configurePhaseCmd[configurePhaseCmdLength];

        ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "%s -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_PREFIX=%s -DEXPAT_SHARED_LIBS=OFF -DCMAKE_VERBOSE_MAKEFILE=%s %s -S . -B build.d", cmakePath, packageInstalledDIR, (installOptions.logLevel >= PPKGLogLevel_verbose) ? "ON" : "OFF", buildConfigureArgs);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        ret = run_cmd(configurePhaseCmd, STDOUT_FILENO);

        if (ret != PPKG_OK) {
            return ret;
        }

        //////////////////////////////////////////////////////////////////////////////

        size_t buildPhaseCmdLength = cmakePathLength + 30U;
        char   buildPhaseCmd[buildPhaseCmdLength];

        ret = snprintf(buildPhaseCmd, buildPhaseCmdLength, "%s --build build.d -- -j%zu", cmakePath, njobs);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        ret = run_cmd(buildPhaseCmd, STDOUT_FILENO);

        if (ret != PPKG_OK) {
            return ret;
        }

        //////////////////////////////////////////////////////////////////////////////

        size_t installPhaseCmdLength = cmakePathLength + 20U;
        char   installPhaseCmd[installPhaseCmdLength];

        ret = snprintf(installPhaseCmd, installPhaseCmdLength, "%s --install build.d", cmakePath);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        ret = run_cmd(installPhaseCmd, STDOUT_FILENO);

        if (ret != PPKG_OK) {
            return ret;
        }
    } else if (buildSystemType == BUILD_SYSTEM_TYPE_CONFIGURE) {
        if (nativePackageID == NATIVE_PACKAGE_ID_PYTHON3) {
            XX xx[5] = {
                { "zlib",   "-lz",   "ZLIB_CFLAGS",  "ZLIB_LIBS"},
                { "libbz2", "-lbz2", "BZIP2_CFLAGS", "BZIP2_LIBS"},
                { "liblzma", "-llzma", "LIBLZMA_CFLAGS", "LIBLZMA_LIBS"},
                { "sqlite3", "-lsqlite3", "LIBSQLITE3_CFLAGS", "LIBSQLITE3_LIBS"},
                { "gdbm", "-lgdbm -lgdbm_compat", "GDBM_CFLAGS", "GDBM_LIBS"},
            };

            for (int i = 0; i < 5; i++) {
                size_t ccflagsSLength = nativePackageInstalledRootDIRLength + strlen(xx[i].name) + 15U;
                char   ccflags[ccflagsSLength];

                ret = snprintf(ccflags, ccflagsSLength, "-I%s/%s/include", nativePackageInstalledRootDIR, xx[i].name);

                if (ret < 0) {
                    perror(NULL);
                    return PPKG_ERROR;
                }

                size_t ldflagsSLength = nativePackageInstalledRootDIRLength + strlen(xx[i].name) + strlen(xx[i].libs) + 10U;
                char   ldflags[ldflagsSLength];

                ret = snprintf(ldflags, ldflagsSLength, "-L%s/%s/lib %s", nativePackageInstalledRootDIR, xx[i].name, xx[i].libs);

                if (ret < 0) {
                    perror(NULL);
                    return PPKG_ERROR;
                }

                if (setenv(xx[i].env1, ccflags, 1) != 0) {
                    perror(xx[i].env1);
                    return PPKG_ERROR;
                }

                if (setenv(xx[i].env2, ldflags, 1) != 0) {
                    perror(xx[i].env2);
                    return PPKG_ERROR;
                }
            }

            if (setenv("LIBS", "-lm", 1) != 0) {
                perror("LIBS");
                return PPKG_ERROR;
            }

            if (unsetenv("PYTHONHOME") < 0) {
                perror("unsetenv PYTHONHOME");
                return PPKG_ERROR;
            }

            if (unsetenv("PYTHONPATH") < 0) {
                perror("unsetenv PYTHONPATH");
                return PPKG_ERROR;
            }

            const char * const CPPFLAGS = getenv("CPPFLAGS");

            if (CPPFLAGS != NULL && CPPFLAGS[0] != '\0') {
                const char * const CCFLAGS = getenv("CFLAGS");

                size_t   newCFLAGSLength = strlen(CCFLAGS) + strlen(CPPFLAGS) + 2U;
                char     newCFLAGS[newCFLAGSLength];
                snprintf(newCFLAGS, newCFLAGSLength, "%s %s", CCFLAGS, CPPFLAGS);

                if (setenv("CFLAGS", newCFLAGS, 1) != 0) {
                    perror("CFLAGS");
                    return PPKG_ERROR;
                }
            }

            printf("CCFLAGS=%s\n", getenv("CFLAGS"));
            printf("LDFLAGS=%s\n", getenv("LDFLAGS"));

            size_t configurePhaseCmdLength = packageInstalledDIRCapacity + nativePackageInstalledRootDIRLength + strlen(buildConfigureArgs) + 60U;
            char   configurePhaseCmd[configurePhaseCmdLength];

            ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "./configure --prefix=%s --with-openssl=%s/openssl %s %s", packageInstalledDIR, nativePackageInstalledRootDIR, (installOptions.logLevel == PPKGLogLevel_silent) ? "--silent" : "", buildConfigureArgs);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            ret = run_cmd(configurePhaseCmd, STDOUT_FILENO);

            if (ret != PPKG_OK) {
                return ret;
            }
        } else if (nativePackageID == NATIVE_PACKAGE_ID_PERL_XML_PARSER) {
            char   perlPath[PATH_MAX];
            size_t perlPathLength = 0U;

            ret = exe_where("perl", perlPath, PATH_MAX, &perlPathLength);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            if (perlPathLength == 0U) {
                fprintf(stderr, "perl command was not found.\n");
                return PPKG_ERROR;
            }

            size_t configurePhaseCmdLength = perlPathLength + (packageInstalledDIRCapacity << 1) + 52U;
            char   configurePhaseCmd[configurePhaseCmdLength];

            ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "%s Makefile.PL EXPATLIBPATH=%s/lib EXPATINCPATH=%s/include", perlPath, packageInstalledDIR, packageInstalledDIR);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            ret = run_cmd(configurePhaseCmd, STDOUT_FILENO);

            if (ret != PPKG_OK) {
                return ret;
            }
        } else if (nativePackageID == NATIVE_PACKAGE_ID_PERL) {
            size_t configurePhaseCmdLength = (packageInstalledDIRCapacity * 3) + 170U;
            char   configurePhaseCmd[configurePhaseCmdLength];

            ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "./Configure -Dprefix=%s -Dman1dir=%s/share/man/man1 -Dman3dir=%s/share/man/man3 -des -Dmake=gmake -Duselargefiles -Duseshrplib -Dusethreads -Dusenm=false -Dusedl=true", packageInstalledDIR, packageInstalledDIR, packageInstalledDIR);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            ret = run_cmd(configurePhaseCmd, STDOUT_FILENO);

            if (ret != PPKG_OK) {
                return ret;
            }
        } else if (nativePackageID == NATIVE_PACKAGE_ID_OPENSSL) {
#if defined (__OpenBSD__)
            // https://github.com/openssl/openssl/issues/19232

            const char * patchPhaseCmd = "/usr/bin/sed -i s/-Wl,-z,defs// Configurations/shared-info.pl";

            size_t  patchPhaseCmdCopyLength = strlen(patchPhaseCmd);
            char    patchPhaseCmdCopy[patchPhaseCmdCopyLength + 1U];
            strncpy(patchPhaseCmdCopy, patchPhaseCmd, patchPhaseCmdCopyLength);

            patchPhaseCmdCopy[patchPhaseCmdCopyLength] = '\0';

            ret = run_cmd(patchPhaseCmdCopy, STDOUT_FILENO);

            if (ret != PPKG_OK) {
                return ret;
            }
#endif

            //////////////////////////////////////////////////////////////////////////////

            size_t configurePhaseCmdLength = (packageInstalledDIRCapacity * 3) + 100U;
            char   configurePhaseCmd[configurePhaseCmdLength];

            ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "./config no-tests no-ssl3 no-ssl3-method no-zlib --prefix=%s --libdir=%s/lib --openssldir=%s/etc/ssl", packageInstalledDIR, packageInstalledDIR, packageInstalledDIR);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            ret = run_cmd(configurePhaseCmd, STDOUT_FILENO);

            if (ret != PPKG_OK) {
                return ret;
            }
        } else {
            size_t configurePhaseCmdLength = packageInstalledDIRCapacity + strlen(buildConfigureArgs) + 32U;
            char   configurePhaseCmd[configurePhaseCmdLength];

            ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "./configure --prefix=%s %s %s", packageInstalledDIR, (installOptions.logLevel == PPKGLogLevel_silent) ? "--silent" : "", buildConfigureArgs);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            ret = run_cmd(configurePhaseCmd, STDOUT_FILENO);

            if (ret != PPKG_OK) {
                return ret;
            }
        }

        //////////////////////////////////////////////////////////////////////////////

        char   gmakePath[PATH_MAX];
        size_t gmakePathLength = 0U;

        ret = exe_where("gmake", gmakePath, PATH_MAX, &gmakePathLength);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (gmakePathLength == 0U) {
            ret = exe_where("make", gmakePath, PATH_MAX, &gmakePathLength);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            if (gmakePathLength == 0U) {
                fprintf(stderr, "neither gmake nor make command was found.\n");
                return PPKG_ERROR;
            }
        }

        //////////////////////////////////////////////////////////////////////////////

        size_t buildPhaseCmdLength = gmakePathLength + 12U;
        char   buildPhaseCmd[buildPhaseCmdLength];

        ret = snprintf(buildPhaseCmd, buildPhaseCmdLength, "%s --jobs=%zu", gmakePath, njobs);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        ret = run_cmd(buildPhaseCmd, STDOUT_FILENO);

        if (ret != PPKG_OK) {
            return ret;
        }

        //////////////////////////////////////////////////////////////////////////////

        size_t installPhaseCmdLength = gmakePathLength + 20U;
        char   installPhaseCmd[installPhaseCmdLength];

        ret = snprintf(installPhaseCmd, installPhaseCmdLength, "%s install", gmakePath);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        ret = run_cmd(installPhaseCmd, STDOUT_FILENO);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    size_t receiptFilePath2Length = packageInstalledDIRCapacity + 14U;
    char   receiptFilePath2[receiptFilePath2Length];

    ret = snprintf(receiptFilePath2, receiptFilePath2Length, "%s/receipt.txt", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    ret = write_to_file(receiptFilePath2, srcSha);

    if (ret != PPKG_OK) {
        return ret;
    }

    if (chdir(nativePackageInstalledRootDIR) != 0) {
        perror(nativePackageInstalledRootDIR);
        return PPKG_ERROR;
    }

    for (;;) {
        if (symlink(packageInstalledSHA, packageName) == 0) {
            fprintf(stderr, "native package '%s' was successfully installed.\n", packageName);
            break;
        } else {
            if (errno == EEXIST) {
                if (lstat(packageName, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        ret = ppkg_rm_r(packageName, installOptions.logLevel >= PPKGLogLevel_verbose);

                        if (ret != PPKG_OK) {
                            return ret;
                        }
                    } else {
                        if (unlink(packageName) != 0) {
                            perror(packageName);
                            return PPKG_ERROR;
                        }
                    }
                }
            } else {
                perror(packageName);
                return PPKG_ERROR;
            }
        }
    }

    ret = export_environment_variables_for_build_tools(packageInstalledDIR, packageInstalledDIRCapacity);

    if (ret != PPKG_OK) {
        return ret;
    }

    return export_environment_variables_for_other_tools(packageInstalledDIR, packageInstalledDIRCapacity);
}

static int install_dependent_packages_via_uppm(
        const char * uppmPackageNames,
        const size_t uppmPackageNamesLength,
        const char * ppkgCoreBinDIR,
        const size_t ppkgCoreBinDIRLength,
        const char * uppmPackageInstalledRootDIR,
        const size_t uppmPackageInstalledRootDIRLength,
        const bool   requestToInstallCmake) {

#if defined (__NetBSD__)
    if (requestToInstallCmake) {
        char cmd[28];

        if (geteuid() == 0) {
            snprintf(cmd, 28, "pkgin -y install cmake");
        } else {
            snprintf(cmd, 28, "sudo pkgin -y install cmake");
        }

        int ret = run_cmd(cmd, STDOUT_FILENO);

        if (ret != PPKG_OK) {
            return ret;
        }
    }
#endif

    //////////////////////////////////////////////////////////////////////////////

    int ret;

    if (false) {
        size_t uppmUpdateCmdLength = ppkgCoreBinDIRLength + 13U;
        char   uppmUpdateCmd[uppmUpdateCmdLength];

        ret = snprintf(uppmUpdateCmd, uppmUpdateCmdLength, "%s/uppm update", ppkgCoreBinDIR);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        ret = run_cmd(uppmUpdateCmd, STDOUT_FILENO);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    size_t uppmInstallCmdLength = ppkgCoreBinDIRLength + uppmPackageNamesLength + 15U;
    char   uppmInstallCmd[uppmInstallCmdLength];

    ret = snprintf(uppmInstallCmd, uppmInstallCmdLength, "%s/uppm install %s", ppkgCoreBinDIR, uppmPackageNames);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    ret = run_cmd(uppmInstallCmd, STDOUT_FILENO);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    char    uppmPackageNamesCopy[uppmPackageNamesLength + 1U];
    strncpy(uppmPackageNamesCopy, uppmPackageNames, uppmPackageNamesLength);

    uppmPackageNamesCopy[uppmPackageNamesLength] = '\0';

    char * uppmPackageName = strtok(uppmPackageNamesCopy, " ");

    while (uppmPackageName != NULL) {
        size_t uppmPackageInstalledDIRCapacity = uppmPackageInstalledRootDIRLength + strlen(uppmPackageName) + 2U;
        char   uppmPackageInstalledDIR[uppmPackageInstalledDIRCapacity];

        ret = snprintf(uppmPackageInstalledDIR, uppmPackageInstalledDIRCapacity, "%s/%s", uppmPackageInstalledRootDIR, uppmPackageName);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        ret = export_environment_variables_for_other_tools(uppmPackageInstalledDIR, uppmPackageInstalledDIRCapacity);

        if (ret != PPKG_OK) {
            return ret;
        }

        if (strcmp(uppmPackageName, "git") == 0) {
            // https://git-scm.com/book/en/v2/Git-Internals-Environment-Variables

            size_t   gitCoreDIRCapacity = uppmPackageInstalledDIRCapacity + 18U;
            char     gitCoreDIR[gitCoreDIRCapacity];
            snprintf(gitCoreDIR, gitCoreDIRCapacity, "%s/libexec/git-core", uppmPackageInstalledDIR);

            if (setenv("GIT_EXEC_PATH" , gitCoreDIR, 1) != 0) {
                perror("GIT_EXEC_PATH");
                return PPKG_ERROR;
            }

            size_t   gitTemplateDIRCapacity = uppmPackageInstalledDIRCapacity + 26U;
            char     gitTemplateDIR[gitTemplateDIRCapacity];
            snprintf(gitTemplateDIR, gitTemplateDIRCapacity, "%s/share/git-core/templates", uppmPackageInstalledDIR);

            if (setenv("GIT_TEMPLATE_DIR" , gitTemplateDIR, 1) != 0) {
                perror("GIT_TEMPLATE_DIR");
                return PPKG_ERROR;
            }
        } else if (strcmp(uppmPackageName, "docbook-xsl") == 0) {
            // http://xmlsoft.org/xslt/xsltproc.html

            size_t   xmlCatalogFilePathCapacity = uppmPackageInstalledDIRCapacity + 13U;
            char     xmlCatalogFilePath[xmlCatalogFilePathCapacity];
            snprintf(xmlCatalogFilePath, xmlCatalogFilePathCapacity, "%s/catalog.xml", uppmPackageInstalledDIR);

            if (setenv("XML_CATALOG_FILES" , xmlCatalogFilePath, 1) != 0) {
                perror("XML_CATALOG_FILES");
                return PPKG_ERROR;
            }
        }

        uppmPackageName = strtok(NULL, " ");
    }

    return PPKG_OK;
}

static int generate_install_shell_script_file(
        const char * packageName,
        const char * installShellScriptFilePath,
        const SysInfo sysinfo,
        const char * ppkgExeFilePath,
        const time_t ts,
        const PPKGInstallOptions installOptions,
        const PPKGFormula * formula,
        const size_t njobs,
        const char * ppkgHomeDIR,
        const char * ppkgCoreDIR,
        const char * ppkgCoreBinDIR,
        const char * ppkgCoreLibexecDIR,
        const char * ppkgDownloadsDIR,
        const char * sessionDIR,
        const char * packageWorkingTopDIR,
        const char * packageWorkingSrcDIR,
        const char * packageWorkingFixDIR,
        const char * packageWorkingResDIR,
        const char * packageWorkingBinDIR,
        const char * packageWorkingLibDIR,
        const char * packageWorkingIncDIR,
        const char * packageWorkingTmpDIR,
        const char * packageInstalledRootDIR,
        const size_t packageInstalledRootDIRCapacity,
        const char * packageInstalledDIR,
        const char * packageMetaInfoDIR,
        const char * recursiveDependentPackageNamesString,
        const size_t recursiveDependentPackageNamesStringSize) {
    KB kbs[] = {
        {"KEEP_SESSION_DIR", installOptions.keepSessionDIR},
        {"BEAR_ENABLED", installOptions.enableBear},
        {"CCACHE_ENABLED", installOptions.enableCcache},
        {"EXPORT_COMPILE_COMMANDS_JSON", installOptions.exportCompileCommandsJson},
        {"PACKAGE_BINBSTD", formula->binbstd},
        {"PACKAGE_SYMLINK", formula->symlink},
        {"PACKAGE_PARALLEL", formula->parallel},
        {"PACKAGE_BUILD_IN_BSCRIPT_DIR", formula->binbstd},
        {"PACKAGE_BUILD_SYSTEM_AUTOGENSH", formula->useBuildSystemAutogen},
        {"PACKAGE_BUILD_SYSTEM_AUTOTOOLS", formula->useBuildSystemAutotools},
        {"PACKAGE_BUILD_SYSTEM_CONFIGURE", formula->useBuildSystemConfigure},
        {"PACKAGE_BUILD_SYSTEM_CMAKE", formula->useBuildSystemCmake},
        {"PACKAGE_BUILD_SYSTEM_XMAKE", formula->useBuildSystemXmake},
        {"PACKAGE_BUILD_SYSTEM_GMAKE", formula->useBuildSystemGmake},
        {"PACKAGE_BUILD_SYSTEM_NINJA", formula->useBuildSystemNinja},
        {"PACKAGE_BUILD_SYSTEM_MESON", formula->useBuildSystemMeson},
        {"PACKAGE_BUILD_SYSTEM_CARGO", formula->useBuildSystemCargo},
        {"PACKAGE_BUILD_SYSTEM_GO", formula->useBuildSystemGolang},
        {NULL,false}
    };

    char * libcName;

    switch(sysinfo.libc) {
        case 1:  libcName = (char*)"glibc"; break;
        case 2:  libcName = (char*)"musl";  break;
        default: libcName = (char*)"";
    }

    const char * linkType;

    switch (installOptions.linkType) {
        case PPKGLinkType_static_full: linkType = (formula->sfslink) ? "static-full" : "static-most"; break;
        case PPKGLinkType_static_most: linkType = "static-most"; break;
        case PPKGLinkType_shared_full: linkType = "shared-full"; break;
        case PPKGLinkType_shared_most: linkType = "shared-most"; break;
    }

    const char * sharedLibrarySuffix;

#if defined (__APPLE__)
    sharedLibrarySuffix = ".dylib";
#else
    sharedLibrarySuffix = ".so";
#endif

    KV kvs[] = {
        {"NATIVE_OS_ARCH", sysinfo.arch },
        {"NATIVE_OS_KIND", sysinfo.kind },
        {"NATIVE_OS_TYPE", sysinfo.type },
        {"NATIVE_OS_CODE", sysinfo.code },
        {"NATIVE_OS_NAME", sysinfo.name },
        {"NATIVE_OS_VERS", sysinfo.vers },
        {"NATIVE_OS_LIBC", libcName },
        {"BUILD_TYPE", installOptions.buildType == PPKGBuildType_release ? "release" : "debug"},
        {"LINK_TYPE", linkType},
        {"INSTALL_LIB", "both"},
        {"STATIC_LIBRARY_SUFFIX", ".a"},
        {"SHARED_LIBRARY_SUFFIX", sharedLibrarySuffix},
        {"PPKG_VERSION", PPKG_VERSION},
        {"PPKG", ppkgExeFilePath},
        {"PPKG_HOME", ppkgHomeDIR},
        {"PPKG_CORE_DIR", ppkgCoreDIR},
        {"PPKG_CORE_BIN_DIR", ppkgCoreBinDIR},
        {"PPKG_DOWNLOADS_DIR", ppkgDownloadsDIR},
        {"PPKG_PACKAGE_INSTALLED_ROOT", packageInstalledRootDIR},
        {"SESSION_DIR", sessionDIR},
        {"RECURSIVE_DEPENDENT_PACKAGE_NAMES", recursiveDependentPackageNamesString},
        {"PACKAGE_FORMULA_FILEPATH", formula->path},
        {"PACKAGE_NAME", packageName},
        {"PACKAGE_SUMMARY", formula->summary},
        {"PACKAGE_VERSION", formula->version},
        {"PACKAGE_LICENSE", formula->license},
        {"PACKAGE_WEB_URL", formula->web_url},
        {"PACKAGE_GIT_URL", formula->git_url},
        {"PACKAGE_GIT_SHA", formula->git_sha},
        {"PACKAGE_GIT_REF", formula->git_ref},
        {"PACKAGE_SRC_URL", formula->src_url},
        {"PACKAGE_SRC_URI", formula->src_uri},
        {"PACKAGE_SRC_SHA", formula->src_sha},
        {"PACKAGE_FIX_URL", formula->fix_url},
        {"PACKAGE_FIX_URI", formula->fix_uri},
        {"PACKAGE_FIX_SHA", formula->fix_sha},
        {"PACKAGE_RES_URL", formula->res_url},
        {"PACKAGE_RES_URI", formula->res_uri},
        {"PACKAGE_RES_SHA", formula->res_sha},
        {"PACKAGE_DEP_PKG", formula->dep_pkg},
        {"PACKAGE_DEP_UPP", formula->dep_upp},
        {"PACKAGE_DEP_PYM", formula->dep_pym},
        {"PACKAGE_DEP_PLM", formula->dep_plm},
        {"PACKAGE_BSYSTEM", formula->bsystem},
        {"PACKAGE_BSCRIPT", formula->bscript},
        {"PACKAGE_PPFLAGS", formula->ppflags},
        {"PACKAGE_CCFLAGS", formula->ccflags},
        {"PACKAGE_XXFLAGS", formula->xxflags},
        {"PACKAGE_LDFLAGS", formula->ldflags},
        {"PACKAGE_INSTALLING_SRC_DIR", packageWorkingSrcDIR},
        {"PACKAGE_INSTALLING_FIX_DIR", packageWorkingFixDIR},
        {"PACKAGE_INSTALLING_RES_DIR", packageWorkingResDIR},
        {"PACKAGE_INSTALLING_BIN_DIR", packageWorkingBinDIR},
        {"PACKAGE_INSTALLING_INC_DIR", packageWorkingIncDIR},
        {"PACKAGE_INSTALLING_LIB_DIR", packageWorkingLibDIR},
        {"PACKAGE_INSTALLING_TMP_DIR", packageWorkingTmpDIR},
        {"PACKAGE_WORKING_DIR", packageWorkingTopDIR},
        {"PACKAGE_INSTALL_DIR", packageInstalledDIR},
        {"PACKAGE_METAINF_DIR", packageMetaInfoDIR},
        {NULL, NULL},
    };

    int fd = open(installShellScriptFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(installShellScriptFilePath);
        return PPKG_ERROR;
    }

    int ret;

    if (installOptions.xtrace) {
        ret = dprintf(fd, "set -x\n");

        if (ret < 0) {
            close(fd);
            return PPKG_ERROR;
        }
    }

    ret = dprintf(fd, "set -e\n\n");

    if (ret < 0) {
        close(fd);
        return PPKG_ERROR;
    }

    ret = dprintf(fd, "TIMESTAMP_UNIX=%zu\n", ts);

    if (ret < 0) {
        close(fd);
        return PPKG_ERROR;
    }

    ret = dprintf(fd, "NATIVE_OS_NCPU=%u\n", sysinfo.ncpu);

    if (ret < 0) {
        close(fd);
        return PPKG_ERROR;
    }

    ret = dprintf(fd, "NATIVE_OS_EUID=%u\n", geteuid());

    if (ret < 0) {
        close(fd);
        return PPKG_ERROR;
    }

    ret = dprintf(fd, "NATIVE_OS_EGID=%u\n", getegid());

    if (ret < 0) {
        close(fd);
        return PPKG_ERROR;
    }

    ret = dprintf(fd, "LOG_LEVEL=%d\n", installOptions.logLevel);

    if (ret < 0) {
        close(fd);
        return PPKG_ERROR;
    }

    ret = dprintf(fd, "BUILD_NJOBS=%zu\n", njobs);

    if (ret < 0) {
        close(fd);
        return PPKG_ERROR;
    }

    ret = dprintf(fd, "PACKAGE_GIT_NTH=%zu\n", formula->git_nth);

    if (ret < 0) {
        close(fd);
        return PPKG_ERROR;
    }

    for (int i = 0; kbs[i].name != NULL; i++) {
        ret = dprintf(fd, "%s=%d\n", kbs[i].name, kbs[i].value);

        if (ret < 0) {
            close(fd);
            return PPKG_ERROR;
        }
    }

    for (int i = 0; kvs[i].name != NULL; i++) {
        ret = dprintf(fd, "%s='%s'\n", kvs[i].name, (kvs[i].value == NULL) ? "" : kvs[i].value);

        if (ret < 0) {
            close(fd);
            return PPKG_ERROR;
        }
    }

    if (formula->bscript == NULL) {
        ret = dprintf(fd, "PACKAGE_BSCRIPT_DIR='%s'\n", packageWorkingSrcDIR);
    } else {
        ret = dprintf(fd, "PACKAGE_BSCRIPT_DIR='%s/%s'\n", packageWorkingSrcDIR, formula->bscript);
    }

    if (ret < 0) {
        close(fd);
        return PPKG_ERROR;
    }

    ret = dprintf(fd, "PACKAGE_BCACHED_DIR='%s/_'\n", packageWorkingSrcDIR);

    if (ret < 0) {
        close(fd);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (recursiveDependentPackageNamesString != NULL) {
        size_t  recursiveDependentPackageNamesStringCopyCapacity = recursiveDependentPackageNamesStringSize + 1U;
        char    recursiveDependentPackageNamesStringCopy[recursiveDependentPackageNamesStringCopyCapacity];
        strncpy(recursiveDependentPackageNamesStringCopy, recursiveDependentPackageNamesString, recursiveDependentPackageNamesStringCopyCapacity);

        char * dependentPackageName = strtok(recursiveDependentPackageNamesStringCopy, " ");

        while (dependentPackageName != NULL) {
            size_t   installedDIRCapacity = packageInstalledRootDIRCapacity + strlen(dependentPackageName) + 1U;
            char     installedDIR[installedDIRCapacity];
            snprintf(installedDIR, installedDIRCapacity, "%s/%s", packageInstalledRootDIR, dependentPackageName);

            size_t   libDIRCapacity = installedDIRCapacity + 4U;
            char     libDIR[libDIRCapacity];
            snprintf(libDIR, libDIRCapacity, "%s/lib", installedDIR);

            size_t   includeDIRCapacity = installedDIRCapacity + 8U;
            char     includeDIR[includeDIRCapacity];
            snprintf(includeDIR, includeDIRCapacity, "%s/include", installedDIR);

            for (int i = 0; ; i++) {
                char c = dependentPackageName[i];

                if (c == '\0') {
                    break;
                }

                if (c == '@' || c == '+' || c == '-' || c == '.') {
                    dependentPackageName[i] = '_';
                }
            }

            KV kvs[3] = {
                {"INSTALL", installedDIR},
                {"INCLUDE", includeDIR},
                {"LIBRARY", libDIR}
            };

            for (int i = 0; i < 3; i++) {
                ret = dprintf(fd, "%s_%s_DIR='%s'\n", dependentPackageName, kvs[i].name, kvs[i].value);

                if (ret < 0) {
                    close(fd);
                    return PPKG_ERROR;
                }
            }

            dependentPackageName = strtok(NULL, " ");
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "dopatch() {\n%s\n}\n\n", formula->dopatch == NULL ? ":" : formula->dopatch);

    if (ret < 0) {
        close(fd);
        return PPKG_ERROR;
    }

    ret = dprintf(fd, "dobuild() {\n%s\n}\n\n", formula->install);

    if (ret < 0) {
        close(fd);
        return PPKG_ERROR;
    }

    ret = dprintf(fd, ". %s/ppkg-install\n", ppkgCoreLibexecDIR);

    if (ret < 0) {
        close(fd);
        return PPKG_ERROR;
    }

    close(fd);

    return PPKG_OK;
}

static int adjust_la_file(const char * filePath, const char * ppkgHomeDIR, size_t ppkgHomeDIRLength) {
    size_t   eLength = ppkgHomeDIRLength + 14U;
    char     e[eLength];
    snprintf(e, eLength, "s|-L%s[^' ]*||g", ppkgHomeDIR);

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (pid == 0) {
        execlp("sed", "sed", "-i", "-e", "s/-Wl,--strip-debug//g", "-e", "s|-R[^' ]*||g", "-e", e, filePath, NULL);
        perror("sed");
        exit(255);
    } else {
        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (childProcessExitStatusCode == 0) {
            return PPKG_OK;
        } else {
            if (WIFEXITED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command 'sed -i -e s/-Wl,--strip-debug//g -e s|-R[^' ]*||g -e %s %s' exit with status code: %d\n", e, filePath, WEXITSTATUS(childProcessExitStatusCode));
            } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command 'sed -i -e s/-Wl,--strip-debug//g -e s|-R[^' ]*||g -e %s %s' killed by signal: %d\n", e, filePath, WTERMSIG(childProcessExitStatusCode));
            } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command 'sed -i -e s/-Wl,--strip-debug//g -e s|-R[^' ]*||g -e %s %s' stopped by signal: %d\n", e, filePath, WSTOPSIG(childProcessExitStatusCode));
            }

            return PPKG_ERROR;
        }
    }
}

static int adjust_la_files(const char * packageInstalledDIR, size_t packageInstalledDIRCapacity, const char * ppkgHomeDIR, size_t ppkgHomeDIRLength) {
    size_t   packageLibDIRLength = packageInstalledDIRCapacity + 5U;
    char     packageLibDIR[packageLibDIRLength];
    snprintf(packageLibDIR, packageLibDIRLength, "%s/lib", packageInstalledDIR);

    struct stat st;

    if (stat(packageLibDIR, &st) != 0) {
        return PPKG_OK;
    }

    DIR * dir = opendir(packageLibDIR);

    if (dir == NULL) {
        perror(packageLibDIR);
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
                perror(packageLibDIR);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t fileNameLength = strlen(dir_entry->d_name);

        if (fileNameLength < 4U) {
            continue;
        }

        char * fileNameSuffix = dir_entry->d_name + fileNameLength - 3U;

        if (strcmp(fileNameSuffix, ".la") == 0) {
            size_t   filePathLength = packageLibDIRLength + fileNameLength  + 2U;
            char     filePath[filePathLength];
            snprintf(filePath, filePathLength, "%s/%s", packageLibDIR, dir_entry->d_name);

            if (stat(filePath, &st) != 0) {
                closedir(dir);
                return PPKG_ERROR;
            }

            if (S_ISREG(st.st_mode)) {
                int ret = adjust_la_file(filePath, ppkgHomeDIR, ppkgHomeDIRLength);

                if (ret != PPKG_OK) {
                    closedir(dir);
                    return ret;
                }
            }
        }
    }
}

static int adjust_pc_files(const char * packageInstalledDIR, size_t packageInstalledDIRCapacity) {
    size_t   packagePkgconfigDIRLength = packageInstalledDIRCapacity + 15U;
    char     packagePkgconfigDIR[packagePkgconfigDIRLength];
    snprintf(packagePkgconfigDIR, packagePkgconfigDIRLength, "%s/lib/pkgconfig", packageInstalledDIR);

    struct stat st;

    if (stat(packagePkgconfigDIR, &st) != 0) {
        return PPKG_OK;
    }

    DIR * dir = opendir(packagePkgconfigDIR);

    if (dir == NULL) {
        perror(packagePkgconfigDIR);
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
                perror(packagePkgconfigDIR);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t fileNameLength = strlen(dir_entry->d_name);

        if (fileNameLength < 4U) {
            continue;
        }

        char * fileNameSuffix = dir_entry->d_name + fileNameLength - 3U;

        if (strcmp(fileNameSuffix, ".pc") == 0) {
            size_t   filePathLength = packagePkgconfigDIRLength + fileNameLength  + 2U;
            char     filePath[filePathLength];
            snprintf(filePath, filePathLength, "%s/%s", packagePkgconfigDIR, dir_entry->d_name);

            if (stat(filePath, &st) != 0) {
                closedir(dir);
                return PPKG_ERROR;
            }

            if (S_ISREG(st.st_mode)) {
                int ret = PPKG_OK;

                if (ret != PPKG_OK) {
                    closedir(dir);
                    return ret;
                }
            }
        }
    }
}

static int adjust_elf_files(const char * packageInstalledDIR, size_t packageInstalledDIRCapacity) {
    return PPKG_OK;
}

static int adjust_macho_files(const char * packageInstalledDIR, size_t packageInstalledDIRCapacity) {
    return PPKG_OK;
}

static int backup_formulas(const char * sessionDIR, const char * packageMetaInfoDIR, const size_t packageMetaInfoDIRCapacity, const char * recursiveDependentPackageNamesString, const size_t recursiveDependentPackageNamesStringSize) {
    size_t   packageInstalledFormulaDIRLength = packageMetaInfoDIRCapacity + 9U;
    char     packageInstalledFormulaDIR[packageInstalledFormulaDIRLength];
    snprintf(packageInstalledFormulaDIR, packageInstalledFormulaDIRLength, "%s/formula", packageMetaInfoDIR);

    if (mkdir(packageInstalledFormulaDIR, S_IRWXU) != 0) {
        perror(packageInstalledFormulaDIR);
        return PPKG_ERROR;
    }

    size_t  recursiveDependentPackageNamesStringCopyCapacity = recursiveDependentPackageNamesStringSize + 1U;
    char    recursiveDependentPackageNamesStringCopy[recursiveDependentPackageNamesStringCopyCapacity];
    strncpy(recursiveDependentPackageNamesStringCopy, recursiveDependentPackageNamesString, recursiveDependentPackageNamesStringCopyCapacity);

    char * packageName = strtok(recursiveDependentPackageNamesStringCopy, " ");

    while (packageName != NULL) {
        size_t packageNameLength = strlen(packageName);

        size_t   fromFilePathLength = strlen(sessionDIR) + packageNameLength  + 6U;
        char     fromFilePath[fromFilePathLength];
        snprintf(fromFilePath, fromFilePathLength, "%s/%s.yml", sessionDIR, packageName);

        size_t   toFilePathLength = packageInstalledFormulaDIRLength + packageNameLength + 6U;
        char     toFilePath[toFilePathLength];
        snprintf(toFilePath, toFilePathLength, "%s/%s.yml", packageInstalledFormulaDIR, packageName);

        int ret = ppkg_copy_file(fromFilePath, toFilePath);

        if (ret != PPKG_OK) {
            return ret;
        }

        packageName = strtok(NULL, " ");
    }

    return PPKG_OK;
}

static int generate_manifest_r(const char * dirPath, size_t offset, FILE * installedManifestFile) {
    if (dirPath == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    size_t dirPathLength = strlen(dirPath);

    if (dirPathLength == 0U) {
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

int generate_manifest(const char * installedDIRPath) {
    size_t installedDIRLength = strlen(installedDIRPath);

    size_t installedManifestFilePathLength = installedDIRLength + 20U;
    char   installedManifestFilePath[installedManifestFilePathLength];
    snprintf(installedManifestFilePath, installedManifestFilePathLength, "%s/.ppkg/MANIFEST.txt", installedDIRPath);

    FILE * installedManifestFile = fopen(installedManifestFilePath, "w");

    if (installedManifestFile == NULL) {
        perror(installedManifestFilePath);
        return PPKG_ERROR;
    }

    int ret = generate_manifest_r(installedDIRPath, installedDIRLength + 1, installedManifestFile);

    fclose(installedManifestFile);

    return ret;
}

static int generate_receipt(const char * packageName, const PPKGFormula * formula, const SysInfo sysinfo, const time_t ts, const char * packageMetaInfoDIR, const size_t packageMetaInfoDIRCapacity) {
    size_t   receiptFilePathLength = packageMetaInfoDIRCapacity + 12U;
    char     receiptFilePath[receiptFilePathLength];
    snprintf(receiptFilePath, receiptFilePathLength, "%s/RECEIPT.yml", packageMetaInfoDIR);

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

    char   buff[2048];
    size_t size = 0U;

    for (;;) {
        size = fread(buff, 1, 2048, formulaFile);

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

    char * libcName;

    switch(sysinfo.libc) {
        case 1:  libcName = (char*)"glibc"; break;
        case 2:  libcName = (char*)"musl";  break;
        default: libcName = (char*)"";
    }

    fprintf(receiptFile, "\nsignature: %s\ntimestamp: %zu\n\n", PPKG_VERSION, ts);

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
        return ppkg_copy_file(fromFilePath, toFilePath);
    }

    return PPKG_OK;
}

static int generate_linker_script(const char * packageWorkingLibDIR, size_t packageWorkingLibDIRLength, const char * filename) {
    size_t filepathCapacity = packageWorkingLibDIRLength + strlen(filename) + 2U;
    char   filepath[filepathCapacity];

    const int ret = snprintf(filepath, filepathCapacity, "%s/%s", packageWorkingLibDIR, filename);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    const int fd = open(filepath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(filepath);
        return PPKG_ERROR;
    }

    const char * const content = "INPUT(-lc)\n";

    const size_t contentLength = strlen(content);

    const ssize_t written = write(fd, content, contentLength);

    if (written == -1) {
        perror(filepath);
        close(fd);
        return PPKG_ERROR;
    }

    close(fd);

    if (written == contentLength) {
        return PPKG_OK;
    } else {
        fprintf(stderr, "file not fully written: %s\n", filepath);
        return PPKG_ERROR;
    }
}

static int copy_dependent_libraries(
        const char * depPackageInstalledDIR,
        const size_t depPackageInstalledDIRLength,
        const char * toDIR,
        const size_t toDIRLength,
        const char * libSuffix,
        const size_t libSuffixLength) {

    size_t fromDIRCapacity = depPackageInstalledDIRLength + 5U;
    char   fromDIR[fromDIRCapacity];

    int ret = snprintf(fromDIR, fromDIRCapacity, "%s/lib", depPackageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    struct stat st;

    if (stat(fromDIR, &st) != 0) {
        return PPKG_OK;
    }

    DIR * dir = opendir(fromDIR);

    if (dir == NULL) {
        perror(fromDIR);
        return PPKG_ERROR;
    }

    char * fileName;
    char * fileNameSuffix;
    size_t fileNameLength;

    struct dirent * dir_entry;

    for (;;) {
        errno = 0;

        dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return PPKG_OK;
            } else {
                perror(fromDIR);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        //puts(dir_entry->d_name);

        fileName = dir_entry->d_name;

        if (strncmp(fileName, "lib", 3) == 0) {
            fileNameLength = strlen(fileName);

            fileNameSuffix = fileName + fileNameLength - libSuffixLength;

            if (strcmp(fileNameSuffix, libSuffix) == 0) {
                size_t   fromFilePathCapacity = fromDIRCapacity + fileNameLength + 2U;
                char     fromFilePath[fromFilePathCapacity];

                ret = snprintf(fromFilePath, fromFilePathCapacity, "%s/%s", fromDIR, fileName);

                if (ret < 0) {
                    perror(NULL);
                    closedir(dir);
                    return PPKG_ERROR;
                }

                size_t   toFilePathCapacity = toDIRLength + fileNameLength + 2U;
                char     toFilePath[toFilePathCapacity];

                ret = snprintf(toFilePath, toFilePathCapacity, "%s/%s", toDIR, fileName);

                if (ret < 0) {
                    perror(NULL);
                    closedir(dir);
                    return PPKG_ERROR;
                }

                ret = ppkg_copy_file(fromFilePath, toFilePath);

                if (ret != PPKG_OK) {
                    closedir(dir);
                    return ret;
                }
            }
        }
    }
}

static int reset_environment_variable_for_compiler_driver_flags(const PPKGToolChain toolchain) {
    const KV flags[8] = {
        { "CFLAGS",   toolchain.ccflags  },
        { "CXXFLAGS", toolchain.cxxflags },
        { "CPPFLAGS", toolchain.cppflags },
        { "LDFLAGS",  toolchain.ldflags  },

        { "CFLAGS_FOR_BUILD",   toolchain.ccflags  },
        { "CXXFLAGS_FOR_BUILD", toolchain.cxxflags },
        { "CPPFLAGS_FOR_BUILD", toolchain.cppflags },
        { "LDFLAGS_FOR_BUILD",  toolchain.ldflags  }
    };

    for (int i = 0; i < 8; i++) {
        const char * name  = flags[i].name;
        const char * value = flags[i].value;

        if (value == NULL) {
            if (unsetenv(name) != 0) {
                perror(name);
                return PPKG_ERROR;
            }
        } else {
            if (setenv(name, value, 1) != 0) {
                perror(name);
                return PPKG_ERROR;
            }
        }
    }

    return PPKG_OK;
}

static int ppkg_install_package(
        const char * packageName,
        const PPKGFormula * formula,
        const PPKGInstallOptions installOptions,
        const PPKGToolChain toolchain,
        const SysInfo sysinfo,
        const char * uppmPackageInstalledRootDIR,
        const size_t uppmPackageInstalledRootDIRLength,
        const char * ppkgExeFilePath,
        const char * ppkgHomeDIR,
        const size_t ppkgHomeDIRLength,
        const char * ppkgCoreDIR,
        const size_t ppkgCoreDIRLength,
        const char * ppkgCoreLibexecDIR,
        const char * ppkgDownloadsDIR,
        const size_t ppkgDownloadsDIRLength,
        const char * sessionDIR,
        const size_t sessionDIRLength,
        const char * recursiveDependentPackageNamesString,
        const size_t recursiveDependentPackageNamesStringSize) {
    fprintf(stderr, "%s=============== Installing%s %s%s%s %s===============%s\n", COLOR_PURPLE, COLOR_OFF, COLOR_GREEN, packageName, COLOR_OFF, COLOR_PURPLE, COLOR_OFF);

    size_t njobs;

    if (formula->parallel) {
        if (installOptions.parallelJobsCount > 0) {
            njobs = installOptions.parallelJobsCount;
        } else {
            njobs = sysinfo.ncpu;
        }
    } else {
        njobs = 1U;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char* unsetenvs[3] = {
        "ACLOCAL_PATH",
        "PKG_CONFIG_PATH",
        "XDG_DATA_DIRS"
    };

    for (int i = 0; i < 3; i++) {
        if (unsetenv(unsetenvs[i]) != 0) {
            perror(unsetenvs[i]);
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    int ret = reset_environment_variable_for_compiler_driver_flags(toolchain);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = export_environment_variables_for_other_tools(ppkgCoreDIR, ppkgCoreDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   packageNameLength = strlen(packageName);

    size_t   packageWorkingTopDIRLength = sessionDIRLength + packageNameLength + 2U;
    char     packageWorkingTopDIR[packageWorkingTopDIRLength];
    snprintf(packageWorkingTopDIR, packageWorkingTopDIRLength, "%s/%s", sessionDIR, packageName);

    struct stat st;

    if (stat(packageWorkingTopDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            int ret = ppkg_rm_r(packageWorkingTopDIR, installOptions.logLevel >= PPKGLogLevel_verbose);

            if (ret != PPKG_OK) {
                return ret;
            }
        } else {
            fprintf(stderr, "%s was expected to be a directory, but it was not.\n", packageWorkingTopDIR);
            return PPKG_ERROR;
        }
    }

    if (mkdir(packageWorkingTopDIR, S_IRWXU) != 0) {
        perror(packageWorkingTopDIR);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   packageWorkingSrcDIRLength = packageWorkingTopDIRLength + 5U;
    char     packageWorkingSrcDIR[packageWorkingSrcDIRLength];
    snprintf(packageWorkingSrcDIR, packageWorkingSrcDIRLength, "%s/src", packageWorkingTopDIR);

    if (mkdir(packageWorkingSrcDIR, S_IRWXU) != 0) {
        perror(packageWorkingSrcDIR);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   packageWorkingFixDIRLength = packageWorkingTopDIRLength + 5U;
    char     packageWorkingFixDIR[packageWorkingFixDIRLength];
    snprintf(packageWorkingFixDIR, packageWorkingFixDIRLength, "%s/fix", packageWorkingTopDIR);

    if (mkdir(packageWorkingFixDIR, S_IRWXU) != 0) {
        perror(packageWorkingFixDIR);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   packageWorkingResDIRLength = packageWorkingTopDIRLength + 5U;
    char     packageWorkingResDIR[packageWorkingResDIRLength];
    snprintf(packageWorkingResDIR, packageWorkingResDIRLength, "%s/res", packageWorkingTopDIR);

    if (mkdir(packageWorkingResDIR, S_IRWXU) != 0) {
        perror(packageWorkingResDIR);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   packageWorkingBinDIRLength = packageWorkingTopDIRLength + 5U;
    char     packageWorkingBinDIR[packageWorkingBinDIRLength];
    snprintf(packageWorkingBinDIR, packageWorkingBinDIRLength, "%s/bin", packageWorkingTopDIR);

    if (mkdir(packageWorkingBinDIR, S_IRWXU) != 0) {
        perror(packageWorkingBinDIR);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   packageWorkingIncDIRLength = packageWorkingTopDIRLength + 9U;
    char     packageWorkingIncDIR[packageWorkingIncDIRLength];
    snprintf(packageWorkingIncDIR, packageWorkingIncDIRLength, "%s/include", packageWorkingTopDIR);

    if (mkdir(packageWorkingIncDIR, S_IRWXU) != 0) {
        perror(packageWorkingIncDIR);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   packageWorkingLibDIRLength = packageWorkingTopDIRLength + 5U;
    char     packageWorkingLibDIR[packageWorkingLibDIRLength];
    snprintf(packageWorkingLibDIR, packageWorkingLibDIRLength, "%s/lib", packageWorkingTopDIR);

    if (mkdir(packageWorkingLibDIR, S_IRWXU) != 0) {
        perror(packageWorkingLibDIR);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   packageWorkingTmpDIRLength = packageWorkingTopDIRLength + 5U;
    char     packageWorkingTmpDIR[packageWorkingTmpDIRLength];
    snprintf(packageWorkingTmpDIR, packageWorkingTmpDIRLength, "%s/tmp", packageWorkingTopDIR);

    if (mkdir(packageWorkingTmpDIR, S_IRWXU) != 0) {
        perror(packageWorkingTmpDIR);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (chdir(packageWorkingTopDIR) != 0) {
        perror(packageWorkingTopDIR);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    // these packages are not relocatable, we should build them from source locally.
    bool requestToBuildRuby     = false;
    bool requestToBuildPerl     = false;
    bool requestToBuildPython3  = false;
    bool requestToBuildLibtool  = false;
    bool requestToBuildAutomake = false;
    bool requestToBuildAutoconf = false;
    bool requestToBuildTexinfo  = false;
    bool requestToBuildHelp2man = false;
    bool requestToBuildIntltool = false;

    size_t depPackageNamesLength = (formula->dep_upp == NULL) ? 0U : strlen(formula->dep_upp);

    bool isNativeOSDarwin;

#if defined (__APPLE__)
    isNativeOSDarwin = true;
#else
    isNativeOSDarwin = false;
#endif

    size_t uppmPackageNamesCapacity = depPackageNamesLength + 100U;
    char   uppmPackageNames[uppmPackageNamesCapacity];
    int    uppmPackageNamesLength = snprintf(uppmPackageNames, 70U, "bash coreutils findutils gsed gawk grep tree pkgconf%s%s", isNativeOSDarwin ? "" : " patchelf", installOptions.enableCcache ? " ccache" : "");

    if (uppmPackageNamesLength < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    bool requestToInstallCmake = false;
    bool requestToInstallGmake = false;
    bool requestToInstallGm4   = false;

    if (formula->dep_upp != NULL) {
        size_t  depPackageNamesCopyCapacity = depPackageNamesLength + 1U;
        char    depPackageNamesCopy[depPackageNamesCopyCapacity];
        strncpy(depPackageNamesCopy, formula->dep_upp, depPackageNamesCopyCapacity);

        char * depPackageName = strtok(depPackageNamesCopy, " ");

        while (depPackageName != NULL) {
            if (strcmp(depPackageName, "ruby") == 0) {
                requestToBuildRuby = true;
                requestToInstallGmake = true;
            } else if (strcmp(depPackageName, "perl") == 0) {
                requestToBuildPerl = true;
                requestToInstallGmake = true;
            } else if (strcmp(depPackageName, "python3") == 0) {
                requestToBuildPython3 = true;
                requestToInstallGmake = true;
                requestToInstallCmake = true;
            } else if (strcmp(depPackageName, "texinfo") == 0) {
                requestToBuildTexinfo = true;
                requestToInstallGmake = true;
            } else if (strcmp(depPackageName, "help2man") == 0) {
                requestToBuildHelp2man = true;
                requestToInstallGmake = true;
            } else if (strcmp(depPackageName, "intltool") == 0) {
                requestToBuildIntltool = true;
                requestToInstallGmake = true;
                requestToInstallCmake = true;
            } else if (strcmp(depPackageName, "libtool") == 0) {
                requestToBuildLibtool = true;
                requestToInstallGmake = true;
                requestToInstallGm4   = true;
            } else if (strcmp(depPackageName, "autoconf") == 0) {
                requestToBuildAutoconf = true;
                requestToInstallGmake = true;
                requestToInstallGm4   = true;
            } else if (strcmp(depPackageName, "automake") == 0) {
                requestToBuildAutomake = true;
                requestToInstallGmake = true;
                requestToInstallGm4   = true;
            } else if (strcmp(depPackageName, "cmake") == 0) {
                requestToInstallCmake = true;
            } else if (strcmp(depPackageName, "gmake") == 0) {
                requestToInstallGmake = true;
            } else {
                int len = snprintf(uppmPackageNames + uppmPackageNamesLength, strlen(depPackageName) + 2U, " %s", depPackageName);

                if (len < 0) {
                    perror(NULL);
                    return PPKG_ERROR;
                }

                uppmPackageNamesLength += len;
            }

            depPackageName = strtok(NULL, " ");
        }

#ifndef __NetBSD__
        if (requestToInstallCmake) {
            strncpy(uppmPackageNames + uppmPackageNamesLength, " cmake", 6U);
            uppmPackageNamesLength += 6U;
        }
#endif

        if (requestToInstallGmake) {
            strncpy(uppmPackageNames + uppmPackageNamesLength, " gmake", 6U);
            uppmPackageNamesLength += 6U;
        }

        if (requestToInstallGm4) {
            strncpy(uppmPackageNames + uppmPackageNamesLength, " gm4", 4U);
            uppmPackageNamesLength += 4U;
        }

        uppmPackageNames[uppmPackageNamesLength] = '\0';
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   ppkgCoreBinDIRLength = ppkgCoreDIRLength + 5U;
    char     ppkgCoreBinDIR[ppkgCoreBinDIRLength];
    snprintf(ppkgCoreBinDIR, ppkgCoreBinDIRLength, "%s/bin", ppkgCoreDIR);

    ret = install_dependent_packages_via_uppm(uppmPackageNames, uppmPackageNamesLength, ppkgCoreBinDIR, ppkgCoreBinDIRLength, uppmPackageInstalledRootDIR, uppmPackageInstalledRootDIRLength, requestToInstallCmake);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    char   m4Path[PATH_MAX];
    size_t m4PathLength = 0U;

    ret = exe_where("m4", m4Path, PATH_MAX, &m4PathLength);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (m4PathLength > 0U) {
        if (setenv("M4", m4Path, 1) != 0) {
            perror("M4");
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    int nativePackageIDArray[20] = {0};
    int nativePackageIDArraySize = 0;

    if (requestToBuildPerl) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_PERL;
        nativePackageIDArraySize++;
    }

    if (requestToBuildLibtool) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_LIBTOOL;
        nativePackageIDArraySize++;
    }

    if (requestToBuildAutoconf) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_AUTOCONF;
        nativePackageIDArraySize++;
    }

    if (requestToBuildAutomake) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_AUTOMAKE;
        nativePackageIDArraySize++;
    }

    if (requestToBuildTexinfo) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_TEXINFO;
        nativePackageIDArraySize++;
    }

    if (requestToBuildHelp2man) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_HELP2MAN;
        nativePackageIDArraySize++;
    }

    if (requestToBuildPython3) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_PYTHON3;
        nativePackageIDArraySize++;
    }

    if (requestToBuildRuby) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_RUBY;
        nativePackageIDArraySize++;
    }

    if (requestToBuildIntltool) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_INTLTOOL;
        nativePackageIDArraySize++;
    }

    size_t   nativePackageInstallingRootDIRLength = packageWorkingTopDIRLength + 8U;
    char     nativePackageInstallingRootDIR[nativePackageInstallingRootDIRLength];
    snprintf(nativePackageInstallingRootDIR, nativePackageInstallingRootDIRLength, "%s/native", packageWorkingTopDIR);

    size_t   nativePackageInstalledRootDIRLength = ppkgHomeDIRLength + 8U;
    char     nativePackageInstalledRootDIR[nativePackageInstalledRootDIRLength];
    snprintf(nativePackageInstalledRootDIR, nativePackageInstalledRootDIRLength, "%s/native", ppkgHomeDIR);

    for (int i = 0; i < nativePackageIDArraySize; i++) {
        ret = reset_environment_variable_for_compiler_driver_flags(toolchain);

        if (ret != PPKG_OK) {
            return ret;
        }

        ret = install_native_package(nativePackageIDArray[i], ppkgDownloadsDIR, ppkgDownloadsDIRLength, nativePackageInstallingRootDIR, nativePackageInstallingRootDIRLength, nativePackageInstalledRootDIR, nativePackageInstalledRootDIRLength, njobs, installOptions);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->dep_pym != NULL) {
        size_t   pipUpgradeCmdLength = nativePackageInstalledRootDIRLength + 57U;
        char     pipUpgradeCmd[pipUpgradeCmdLength];
        snprintf(pipUpgradeCmd, pipUpgradeCmdLength, "%s/python3/bin/pip3 install --upgrade pip setuptools wheel", nativePackageInstalledRootDIR);

        ret = run_cmd(pipUpgradeCmd, STDOUT_FILENO);

        if (ret != PPKG_OK) {
            return ret;
        }

        size_t   pipInstallCmdLength = nativePackageInstalledRootDIRLength + strlen(formula->dep_pym) + 37U;
        char     pipInstallCmd[pipInstallCmdLength];
        snprintf(pipInstallCmd, pipInstallCmdLength, "%s/python3/bin/pip3 install --upgrade %s", nativePackageInstalledRootDIR, formula->dep_pym);

        ret = run_cmd(pipInstallCmd, STDOUT_FILENO);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->dep_plm != NULL) {
        size_t   cpanInstallCmdLength = ppkgHomeDIRLength + strlen(formula->dep_plm) + 24U;
        char     cpanInstallCmd[cpanInstallCmdLength];
        snprintf(cpanInstallCmd, cpanInstallCmdLength, "%s/native/perl/bin/cpan %s", ppkgHomeDIR, formula->dep_plm);

        ret = run_cmd(cpanInstallCmd, STDOUT_FILENO);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemCargo) {
        ret = setup_rust_toolchain(installOptions, sessionDIR, sessionDIRLength);

        if (ret != PPKG_OK) {
            return ret;
        }

        // https://docs.rs/backtrace/latest/backtrace/
        if (setenv("RUST_BACKTRACE", "1", 1) != 0) {
            perror("RUST_BACKTRACE");
            return PPKG_ERROR;
        }

        char     ns[4];
        snprintf(ns, 4, "%zu", njobs);

        // https://doc.rust-lang.org/cargo/reference/environment-variables.html
        if (setenv("CARGO_BUILD_JOBS", ns, 1) != 0) {
            perror("CARGO_BUILD_JOBS");
            return PPKG_ERROR;
        }

        char rustTarget[64] = {0};

#if defined (__linux__)
        const char * libcName;

        switch(sysinfo.libc) {
            case 1:  libcName = "gnu" ; break;
            case 2:  libcName = "musl"; break;
            default: libcName = "";
        }

        ret = snprintf(rustTarget, 64, "%s-unknown-linux-%s", sysinfo.arch, libcName);
#else
        const char * arch;

        if (strcmp(sysinfo.arch, "amd64") == 0) {
            arch = "x86_64";
        } else {
            arch = sysinfo.arch;
        }

        ret = snprintf(rustTarget, 64, "%s-unknown-%s", arch, sysinfo.kind);
#endif

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (setenv("RUST_TARGET", rustTarget, 1) != 0) {
            perror("RUST_TARGET");
            return PPKG_ERROR;
        }

        size_t i = 0U;

        for (;;) {
            if (rustTarget[i] == '\0') {
                break;
            }

            if (rustTarget[i] == '-') {
                rustTarget[i] = '_';
            }

            if (rustTarget[i] >= 'a' && rustTarget[i] <= 'z') {
                rustTarget[i] -= 32;
            }

            i++;
        }

        // https://doc.rust-lang.org/cargo/reference/environment-variables.html
        // https://doc.rust-lang.org/cargo/reference/config.html#targettriplelinker
        size_t   linkerLength = i + 21U;
        char     linker[linkerLength];
        snprintf(linker, linkerLength, "CARGO_TARGET_%s_LINKER", rustTarget);

        if (setenv(linker, toolchain.cc, 1) != 0) {
            perror(linker);
            return PPKG_ERROR;
        }

        // https://doc.rust-lang.org/cargo/reference/config.html#buildrustflags
        // we want to use RUSTFLAGS
        if (unsetenv("CARGO_ENCODED_RUSTFLAGS") != 0) {
            perror("CARGO_ENCODED_RUSTFLAGS");
            return PPKG_ERROR;
        }

        size_t   rustFlagsCapacity = strlen(toolchain.cc) + 10U;
        char     rustFlags[rustFlagsCapacity];
        snprintf(rustFlags, rustFlagsCapacity, "-Clinker=%s", toolchain.cc);

        if (setenv("RUSTFLAGS", rustFlags, 1) != 0) {
            perror("RUSTFLAGS");
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    ///                        below is for target                             ///
    //////////////////////////////////////////////////////////////////////////////

    ret = reset_environment_variable_for_compiler_driver_flags(toolchain);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageInstalledRootDIRCapacity = ppkgHomeDIRLength + 11U;
    char   packageInstalledRootDIR[packageInstalledRootDIRCapacity];

    ret = snprintf(packageInstalledRootDIR, packageInstalledRootDIRCapacity, "%s/installed", ppkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t cppflagsCapacity = packageWorkingIncDIRLength + 3U;
    char   cppflags[cppflagsCapacity];

    ret = snprintf(cppflags, cppflagsCapacity, "-I%s", packageWorkingIncDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (setenv("CPPFLAGS", cppflags, 1) != 0) {
        perror("CPPFLAGS");
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    bool flag = false;

    if (installOptions.linkType == PPKGLinkType_static_full) {
        if (formula->sfslink) {
            flag = true;
        } else {
            fprintf(stderr, "user request to create fully statically linked executable, but package '%s' DO NOT support it, so we will downgrade to mostly statically linked executable.\n", packageName);
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t ldflagsCapacity = strlen(toolchain.ldflags) + packageWorkingLibDIRLength + packageInstalledRootDIRCapacity + packageNameLength + 40U;
    char   ldflags[ldflagsCapacity];

    ret = snprintf(ldflags, ldflagsCapacity, "%s%s -L%s -Wl,-rpath,%s/%s/lib", flag ? "--static " : "", toolchain.ldflags, packageWorkingLibDIR, packageInstalledRootDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (setenv("LDFLAGS", ldflags, 1) != 0) {
        perror("LDFLAGS");
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * libSuffix;

    if (installOptions.linkType == PPKGLinkType_static_full || installOptions.linkType == PPKGLinkType_static_most) {
        libSuffix = ".a";
    } else {
#if defined (__APPLE__)
        libSuffix = ".dylib";
#else
        libSuffix = ".so";
#endif
    }

    const size_t libSuffixLength = strlen(libSuffix);

    //////////////////////////////////////////////////////////////////////////////

    if (recursiveDependentPackageNamesString != NULL) {
        size_t  recursiveDependentPackageNamesStringCopyCapacity = recursiveDependentPackageNamesStringSize + 1U;
        char    recursiveDependentPackageNamesStringCopy[recursiveDependentPackageNamesStringCopyCapacity];
        strncpy(recursiveDependentPackageNamesStringCopy, recursiveDependentPackageNamesString, recursiveDependentPackageNamesStringCopyCapacity);

        char * dependentPackageName = strtok(recursiveDependentPackageNamesStringCopy, " ");

        while (dependentPackageName != NULL) {
            size_t installedDIRCapacity = packageInstalledRootDIRCapacity + strlen(dependentPackageName) + 1U;
            char   installedDIR[installedDIRCapacity];

            ret = snprintf(installedDIR, installedDIRCapacity, "%s/%s", packageInstalledRootDIR, dependentPackageName);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            ret = export_environment_variables_for_build_tools(installedDIR, installedDIRCapacity);

            if (ret != PPKG_OK) {
                return ret;
            }

            ret = export_environment_variables_for_other_tools(installedDIR, installedDIRCapacity);

            if (ret != PPKG_OK) {
                return ret;
            }

            ret = copy_dependent_libraries(installedDIR, installedDIRCapacity, packageWorkingLibDIR, packageWorkingLibDIRLength, libSuffix, libSuffixLength);

            if (ret != PPKG_OK) {
                return ret;
            }

            dependentPackageName = strtok(NULL, " ");
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemCmake) {
        // https://cmake.org/cmake/help/latest/envvar/CMAKE_GENERATOR.html
        if (setenv("CMAKE_GENERATOR", formula->useBuildSystemNinja ? "Ninja" : "Unix Makefiles", 1) != 0) {
            perror("CMAKE_GENERATOR");
            return PPKG_ERROR;
        }

        // https://cmake.org/cmake/help/latest/envvar/CMAKE_BUILD_TYPE.html
        if (setenv("CMAKE_BUILD_TYPE", installOptions.buildType == PPKGBuildType_release ? "Release" : "Debug", 1) != 0) {
            perror("CMAKE_BUILD_TYPE");
            return PPKG_ERROR;
        }

        char     ns[4];
        snprintf(ns, 4, "%zu", njobs);

        // https://cmake.org/cmake/help/latest/envvar/CMAKE_BUILD_PARALLEL_LEVEL.html
        if (setenv("CMAKE_BUILD_PARALLEL_LEVEL", ns, 1) != 0) {
            perror("CMAKE_BUILD_PARALLEL_LEVEL");
            return PPKG_ERROR;
        }

        // https://cmake.org/cmake/help/latest/envvar/CMAKE_EXPORT_COMPILE_COMMANDS.html
        if (setenv("CMAKE_EXPORT_COMPILE_COMMANDS", installOptions.exportCompileCommandsJson ? "ON" : "OFF", 1) != 0) {
            perror("CMAKE_EXPORT_COMPILE_COMMANDS");
            return PPKG_ERROR;
        }

        // https://cmake.org/cmake/help/latest/manual/cmake-env-variables.7.html#manual:cmake-env-variables(7)

        const char* cmakeenvs[] = {
            "CMAKE_PREFIX_PATH",
            "CMAKE_APPLE_SILICON_PROCESSOR",
            "CMAKE_BUILD_TYPE",
            "CMAKE_CONFIGURATION_TYPES",
            "CMAKE_CONFIG_TYPE",
            "CMAKE_GENERATOR_INSTANCE",
            "CMAKE_GENERATOR_PLATFORM",
            "CMAKE_GENERATOR_TOOLSET",
            "CMAKE_INSTALL_MODE",
            "CMAKE_C_COMPILER_LAUNCHER",
            "CMAKE_C_LINKER_LAUNCHER",
            "CMAKE_CXX_COMPILER_LAUNCHER",
            "CMAKE_CXX_LINKER_LAUNCHER",
            "CMAKE_MSVCIDE_RUN_PATH",
            "CMAKE_NO_VERBOSE",
            "CMAKE_OSX_ARCHITECTURES",
            "CMAKE_TOOLCHAIN_FILE",
            "DESTDIR",
            "CTEST_INTERACTIVE_DEBUG_MODE",
            "CTEST_OUTPUT_ON_FAILURE",
            "CTEST_PARALLEL_LEVEL",
            "CTEST_PROGRESS_OUTPUT",
            "CTEST_USE_LAUNCHERS_DEFAULT",
            "DASHBOARD_TEST_FROM_CTEST",
            NULL
        };

        for (int i = 0; ; i++) {
            const char * name = cmakeenvs[i];

            if (name == NULL) {
                break;
            }

            if (unsetenv(cmakeenvs[i]) != 0) {
                perror(cmakeenvs[i]);
                return PPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemGolang) {
        // https://pkg.go.dev/cmd/cgo
        KV goenvs[6] = {
            { "GO111MODULE",  "auto" },
            { "CGO_ENABLED",  "0" },
            { "CGO_CFLAGS",   getenv("CFLAGS") },
            { "CGO_CXXFLAGS", getenv("CXXFLAGS") },
            { "CGO_CPPFLAGS", getenv("CPPFLAGS") },
            { "CGO_LDFLAGS",  getenv("LDFLAGS") },
        };

        for (int i = 0; i < 6; i++) {
            const char * name  = goenvs[i].name;
            const char * value = goenvs[i].value;

            if (value == NULL) {
                value = "";
            }

            if (setenv(name, value, 1) != 0) {
                perror(name);
                return PPKG_ERROR;
            }
        }

        // https://golang.org/doc/install/source#environment

        if (unsetenv("GOOS") != 0) {
            perror("GOOS");
            return PPKG_ERROR;
        }

        if (unsetenv("GOARCH") != 0) {
            perror("GOARCH");
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemCargo) {
        // https://libraries.io/cargo/cc
        // https://crates.io/crates/cc
        // https://docs.rs/cc/latest/cc/
        // https://github.com/alexcrichton/cc-rs

        const char *    cflags = getenv("CFLAGS");
        const char *  cxxflags = getenv("CXXFLAGS");
        const char *  cppflags = getenv("CPPFLAGS");

        if (cppflags == NULL) {
            cppflags = "";
        }

        const size_t cppflagsLength = strlen(cppflags);

        size_t   hostCFLAGSCapacity = strlen(cflags) + cppflagsLength + 2U;
        char     hostCFLAGS[hostCFLAGSCapacity];
        snprintf(hostCFLAGS, hostCFLAGSCapacity, "%s %s", cflags, cppflags);

        size_t   hostCXXFLAGSCapacity = strlen(cxxflags) + cppflagsLength + 2U;
        char     hostCXXFLAGS[hostCXXFLAGSCapacity];
        snprintf(hostCXXFLAGS, hostCXXFLAGSCapacity, "%s %s", cxxflags, cppflags);

        KV envs[10] = {
            { "HOST_CC",         toolchain.cc },
            { "HOST_CFLAGS",     hostCFLAGS },
            { "HOST_CXX",        toolchain.cxx },
            { "HOST_CXXFLAGS",   hostCXXFLAGS },
            { "HOST_AR",         toolchain.ar },

            { "TARGET_CC",       toolchain.cc },
            { "TARGET_CFLAGS",   hostCFLAGS },
            { "TARGET_CXX",      toolchain.cxx },
            { "TARGET_CXXFLAGS", hostCXXFLAGS },
            { "TARGET_AR",       toolchain.ar }
        };

        for (int i = 0; i < 10; i++) {
            if (setenv(envs[i].name, envs[i].value, 1) != 0) {
                perror(envs[i].name);
                return PPKG_ERROR;
            }
        }

        const char * LDFLAGS = getenv("LDFLAGS");

        if (LDFLAGS != NULL && LDFLAGS[0] != '\0') {
            size_t  ldflagsCopyCapacity = strlen(LDFLAGS) + 1U;
            char    ldflagsCopy[ldflagsCopyCapacity];
            strncpy(ldflagsCopy, LDFLAGS, ldflagsCopyCapacity);

            char * item = strtok(ldflagsCopy, " ");

            while (item != NULL) {
                const char * const RUSTFLAGS = getenv("RUSTFLAGS");

                size_t   newRUSTFLAGSCapacity = strlen(RUSTFLAGS) + strlen(item) + 13U;
                char     newRUSTFLAGS[newRUSTFLAGSCapacity];
                snprintf(newRUSTFLAGS, newRUSTFLAGSCapacity, "%s -Clink-arg=%s", RUSTFLAGS, item);

                // https://doc.rust-lang.org/rustc/codegen-options/index.html#link-arg
                if (setenv("RUSTFLAGS", newRUSTFLAGS, 1) != 0) {
                    perror("RUSTFLAGS");
                    return PPKG_ERROR;
                }

                item = strtok(NULL, " ");
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    time_t ts = time(NULL);

    size_t   strBufSize = packageNameLength + 50U;
    char     strBuf[strBufSize];
    snprintf(strBuf, strBufSize, "%s:%zu:%u", packageName, ts, getpid());

    char packageInstalledSHA[65] = {0};

    ret = sha256sum_of_string(packageInstalledSHA, strBuf);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t   packageInstalledDIRCapacity = packageInstalledRootDIRCapacity + 66U;
    char     packageInstalledDIR[packageInstalledDIRCapacity];
    snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", packageInstalledRootDIR, packageInstalledSHA);

    size_t   packageMetaInfoDIRCapacity = packageInstalledDIRCapacity + 6U;
    char     packageMetaInfoDIR[packageMetaInfoDIRCapacity];
    snprintf(packageMetaInfoDIR, packageMetaInfoDIRCapacity, "%s/.ppkg", packageInstalledDIR);

    //////////////////////////////////////////////////////////////////////////////

    size_t   installShellScriptFilePathCapacity = packageWorkingBinDIRLength + 12U;
    char     installShellScriptFilePath[installShellScriptFilePathCapacity];
    snprintf(installShellScriptFilePath, installShellScriptFilePathCapacity, "%s/install.sh", packageWorkingBinDIR);

    ret = generate_install_shell_script_file(packageName, installShellScriptFilePath, sysinfo, ppkgExeFilePath, ts, installOptions, formula, njobs, ppkgHomeDIR, ppkgCoreDIR, ppkgCoreBinDIR, ppkgCoreLibexecDIR, ppkgDownloadsDIR, sessionDIR, packageWorkingTopDIR, packageWorkingSrcDIR, packageWorkingFixDIR, packageWorkingResDIR, packageWorkingBinDIR, packageWorkingLibDIR, packageWorkingIncDIR, packageWorkingTmpDIR, packageInstalledRootDIR, packageInstalledRootDIRCapacity, packageInstalledDIR, packageMetaInfoDIR, recursiveDependentPackageNamesString, recursiveDependentPackageNamesStringSize);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->src_url == NULL) {
        const char * remoteRef;

        if (formula->git_sha == NULL) {
            remoteRef = (formula->git_ref == NULL) ? "HEAD" : formula->git_ref;
        } else {
            remoteRef = formula->git_sha;
        }

        ret = ppkg_git_sync(packageWorkingSrcDIR, formula->git_url, remoteRef, "refs/remotes/origin/master", "master");

        if (ret != PPKG_OK) {
            return ret;
        }
    } else {
        if (formula->src_is_dir) {
            char * srcDIR = &formula->src_url[6];
            size_t srcDIRLength = strlen(srcDIR);

            size_t cmdCapacity = srcDIRLength + packageWorkingSrcDIRLength + 10U;
            char   cmd[cmdCapacity];

            ret = snprintf(cmd, cmdCapacity, "cp -r %s/. %s", srcDIR, packageWorkingSrcDIR);

            if (ret < 0) {
                perror(NULL);
                return PPKG_ERROR;
            }

            ret = run_cmd(cmd, STDOUT_FILENO);

            if (ret != PPKG_OK) {
                return ret;
            }
        } else {
            ret = download_via_http(formula->src_url, formula->src_uri, formula->src_sha, ppkgDownloadsDIR, ppkgDownloadsDIRLength, packageWorkingSrcDIR, packageWorkingSrcDIRLength, installOptions.verbose_net);

            if (ret != PPKG_OK) {
                return ret;
            }
        }
    }

    if (formula->fix_url != NULL) {
        ret = download_via_http(formula->fix_url, formula->fix_uri, formula->fix_sha, ppkgDownloadsDIR, ppkgDownloadsDIRLength, packageWorkingFixDIR, packageWorkingFixDIRLength, installOptions.verbose_net);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    if (formula->res_url != NULL) {
        ret = download_via_http(formula->res_url, formula->res_uri, formula->res_sha, ppkgDownloadsDIR, ppkgDownloadsDIRLength, packageWorkingResDIR, packageWorkingResDIRLength, installOptions.verbose_net);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    // override the default search directory (usually /usr/lib/pkgconfig:/usr/share/pkgconfig)
    // because we only want to use our own
    if (setenv("PKG_CONFIG_LIBDIR", packageWorkingLibDIR, 1) != 0) {
        perror("PKG_CONFIG_LIBDIR");
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

#if defined (__NetBSD__)
    ret = generate_linker_script(packageWorkingLibDIR, packageWorkingLibDIRLength, "libdl.a");

    if (ret != PPKG_OK) {
        return ret;
    }
#elif defined (__OpenBSD__)
    // https://github.com/mesonbuild/meson/issues/5390

    ret = generate_linker_script(packageWorkingLibDIR, packageWorkingLibDIRLength, "libdl.a");

    if (ret != PPKG_OK) {
        return ret;
    }

    ret = generate_linker_script(packageWorkingLibDIR, packageWorkingLibDIRLength, "librt.a");

    if (ret != PPKG_OK) {
        return ret;
    }

    ret = generate_linker_script(packageWorkingLibDIR, packageWorkingLibDIRLength, "libcrypt.a");

    if (ret != PPKG_OK) {
        return ret;
    }
#endif

    //////////////////////////////////////////////////////////////////////////////

    if (installOptions.dryrun) {
        const char * const SHELL = getenv("SHELL");

        if (SHELL == NULL) {
            fprintf(stderr, "SHELL environment variable is not set.\n");
            return PPKG_ERROR;
        }

        if (SHELL[0] == '\0') {
            fprintf(stderr, "SHELL environment variable 's value should be a non-empty string.\n");
            return PPKG_ERROR;
        }

        execl(SHELL, SHELL, NULL);

        perror(SHELL);

        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t bashCmdCapacity = installShellScriptFilePathCapacity + 10U;
    char   bashCmd[bashCmdCapacity];

    ret = snprintf(bashCmd, bashCmdCapacity, "/bin/sh %s", installShellScriptFilePath);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    ret = run_cmd(bashCmd, STDOUT_FILENO);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (stat(packageInstalledDIR, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "nothing is installed.\n");
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char* a[2] = { ".crates.toml", ".crates2.json" };

    for (int i = 0; i < 2; i++) {
        size_t filePathCapacity = packageInstalledDIRCapacity + strlen(a[i]) + 2U;
        char   filePath[filePathCapacity];

        ret = snprintf(filePath, filePathCapacity, "%s/%s", packageInstalledDIR, a[i]);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (stat(filePath, &st) == 0) {
            if (unlink(filePath) != 0) {
                perror(filePath);
                return PPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (mkdir(packageMetaInfoDIR, S_IRWXU) != 0) {
        perror(packageMetaInfoDIR);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////
    // install dependency graph files

    if (formula->dep_pkg != NULL) {
        const char* item[4] = { "dependencies.dot", "dependencies.box", "dependencies.svg", "dependencies.png" };

        for (int i = 0; i < 4; i++) {
            ret = install_files_to_metainfo_dir(st, packageWorkingTopDIR, packageWorkingTopDIRLength, packageMetaInfoDIR, packageMetaInfoDIRCapacity, item[i], strlen(item[i]));

            if (ret != PPKG_OK) {
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // install config.log

    size_t packageBuildScriptDIRBufCapacity = packageWorkingSrcDIRLength + (formula->bscript == NULL ? 0U : strlen(formula->bscript)) + 2U;
    char   packageBuildScriptDIRBuf[packageBuildScriptDIRBufCapacity];

    const char * packageBuildScriptDIR;

    if (formula->bscript == NULL) {
        packageBuildScriptDIR = packageWorkingSrcDIR;
    } else {
        ret = snprintf(packageBuildScriptDIRBuf, packageBuildScriptDIRBufCapacity, "%s/%s", packageWorkingSrcDIR, formula->bscript);

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        packageBuildScriptDIR = packageBuildScriptDIRBuf;
    }

    const char* b[2] = { packageWorkingTmpDIR, packageBuildScriptDIR };

    for (int i = 0; i < 2; i++) {
        size_t   fromFilePathCapacity = strlen(b[i]) + 12U;
        char     fromFilePath[fromFilePathCapacity];
        snprintf(fromFilePath, fromFilePathCapacity, "%s/config.log", b[i]);

        if (stat(fromFilePath, &st) == 0) {
            size_t   toFilePathCapacity = packageMetaInfoDIRCapacity + 12U;
            char     toFilePath[toFilePathCapacity];
            snprintf(toFilePath, toFilePathCapacity, "%s/config.log", packageMetaInfoDIR);

            ret = ppkg_rename_or_copy_file(fromFilePath, toFilePath);

            if (ret != PPKG_OK) {
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // install compile_commands.json

    if (installOptions.exportCompileCommandsJson) {
        for (int i = 0; i < 2; i++) {
            size_t   fromFilePathCapacity = strlen(b[i]) + 23U;
            char     fromFilePath[fromFilePathCapacity];
            snprintf(fromFilePath, fromFilePathCapacity, "%s/compile_commands.json", b[i]);

            if (stat(fromFilePath, &st) == 0) {
                size_t   toFilePathCapacity = packageMetaInfoDIRCapacity + 23U;
                char     toFilePath[toFilePathCapacity];
                snprintf(toFilePath, toFilePathCapacity, "%s/compile_commands.json", packageMetaInfoDIR);

                ret = ppkg_rename_or_copy_file(fromFilePath, toFilePath);

                if (ret != PPKG_OK) {
                    return ret;
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    
    const char* x[12] = { "AUTHORS", "LICENSE", "COPYING", "FAQ", "TODO", "NEWS", "THANKS", "CHANGELOG", "CHANGES", "README", "CONTRIBUTORS", "CONTRIBUTING" };

    const char* y[3] = { "", ".md", ".rst" };

    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 3; j++) {
            size_t   itemCapacity = strlen(x[i]) + strlen(y[j]) + 1U;
            char     item[itemCapacity];
            snprintf(item, itemCapacity, "%s%s", x[i], y[j]);

            ret = install_files_to_metainfo_dir(st, packageWorkingSrcDIR, packageWorkingSrcDIRLength, packageMetaInfoDIR, packageMetaInfoDIRCapacity, item, itemCapacity);

            if (ret != PPKG_OK) {
                return ret;
            }
        }
    }

    if (recursiveDependentPackageNamesString != NULL) {
        ret = backup_formulas(sessionDIR, packageMetaInfoDIR, packageMetaInfoDIRCapacity, recursiveDependentPackageNamesString, recursiveDependentPackageNamesStringSize);

        if (ret != PPKG_OK) {
            return ret;
        }
    }

    ret = generate_manifest(packageInstalledDIR);

    if (ret != PPKG_OK) {
        return ret;
    }

    ret = generate_receipt(packageName, formula, sysinfo, ts, packageMetaInfoDIR, packageMetaInfoDIRCapacity);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t treeCmdCapacity = uppmPackageInstalledRootDIRLength + packageInstalledDIRCapacity + 30U;
    char   treeCmd[treeCmdCapacity];

    ret = snprintf(treeCmd, treeCmdCapacity, "%s/tree/bin/tree -a --dirsfirst %s", uppmPackageInstalledRootDIR, packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    ret = run_cmd(treeCmd, STDOUT_FILENO);

    if (ret != PPKG_OK) {
        return ret;
    }

    if (chdir(packageInstalledRootDIR) != 0) {
        perror(packageInstalledRootDIR);
        return PPKG_ERROR;
    }

    for (;;) {
        if (symlink(packageInstalledSHA, packageName) == 0) {
            fprintf(stderr, "package '%s' was successfully installed.\n", packageName);
            break;
        } else {
            if (errno == EEXIST) {
                if (lstat(packageName, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        ret = ppkg_rm_r(packageName, installOptions.logLevel >= PPKGLogLevel_verbose);

                        if (ret != PPKG_OK) {
                            return ret;
                        }
                    } else {
                        if (unlink(packageName) != 0) {
                            perror(packageName);
                            return PPKG_ERROR;
                        }
                    }
                }
            } else {
                perror(packageName);
                return PPKG_ERROR;
            }
        }
    }

    if (installOptions.keepSessionDIR) {
        return PPKG_OK;
    } else {
        return ppkg_rm_r(packageWorkingTopDIR, installOptions.logLevel >= PPKGLogLevel_verbose);
    }
}

typedef struct {
    char * packageName;
    PPKGFormula * formula;
} PPKGPackage;

static int getRecursiveDependentPackageNamesStringBuffer(char * packageName, PPKGPackage ** packageSet, size_t packageSetSize, char ** precursiveDependentPackageNamesStringBuffer, size_t * precursiveDependentPackageNamesStringBufferSize, size_t * precursiveDependentPackageNamesStringBufferCapacity) {
    PPKGPackage * package = NULL;

    for (size_t i = 0U; i < packageSetSize; i++) {
        if (strcmp(packageSet[i]->packageName, packageName) == 0) {
            package = packageSet[i];
            break;
        }
    }

    PPKGFormula * formula = package->formula;

    if (formula->dep_pkg == NULL) {
        return PPKG_OK;
    }

    size_t   packageNameStackCapacity = 8U;
    size_t   packageNameStackSize    = 0U;
    char * * packageNameStack = (char**)malloc(8 * sizeof(char*));

    if (packageNameStack == NULL) {
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    size_t  depPackageNamesLength = strlen(formula->dep_pkg);

    size_t  depPackageNamesCopyCapacity = depPackageNamesLength + 1U;
    char    depPackageNamesCopy[depPackageNamesCopyCapacity];
    strncpy(depPackageNamesCopy, formula->dep_pkg, depPackageNamesCopyCapacity);

    char * depPackageName = strtok(depPackageNamesCopy, " ");

    while (depPackageName != NULL) {
        if (packageNameStackSize == packageNameStackCapacity) {
            char ** p = (char**)realloc(packageNameStack, (packageNameStackCapacity + 8U) * sizeof(char*));

            if (p == NULL) {
                free(packageNameStack);
                packageNameStack = NULL;

                return PPKG_ERROR_MEMORY_ALLOCATE;
            }

            packageNameStack = p;
            packageNameStackCapacity += 8U;
        }

        packageNameStack[packageNameStackSize] = depPackageName;
        packageNameStackSize++;
        depPackageName = strtok(NULL, " ");
    }

    ////////////////////////////////////////////////////////////////

    char * recursiveDependentPackageNamesStringBuffer        = (*precursiveDependentPackageNamesStringBuffer);
    size_t recursiveDependentPackageNamesStringBufferSize    = (*precursiveDependentPackageNamesStringBufferSize);
    size_t recursiveDependentPackageNamesStringBufferCapacity = (*precursiveDependentPackageNamesStringBufferCapacity);

    ////////////////////////////////////////////////////////////////

    while (packageNameStackSize != 0U) {
        char * packageName = packageNameStack[packageNameStackSize - 1U];
        packageNameStack[packageNameStackSize - 1U] = NULL;
        packageNameStackSize--;

        ////////////////////////////////////////////////////////////////

        if (recursiveDependentPackageNamesStringBuffer != NULL) {
            bool alreadyInRecursiveDependentPackageNamesStringBuffer = false;

            size_t  recursiveDependentPackageNamesStringBufferCopyCapacity = recursiveDependentPackageNamesStringBufferSize + 1U;
            char    recursiveDependentPackageNamesStringBufferCopy[recursiveDependentPackageNamesStringBufferCopyCapacity];
            strncpy(recursiveDependentPackageNamesStringBufferCopy, recursiveDependentPackageNamesStringBuffer, recursiveDependentPackageNamesStringBufferCopyCapacity);

            char * dependentPackageName = strtok(recursiveDependentPackageNamesStringBufferCopy, " ");

            while (dependentPackageName != NULL) {
                if (strcmp(dependentPackageName, packageName) == 0) {
                    alreadyInRecursiveDependentPackageNamesStringBuffer = true;
                    break;
                }
                dependentPackageName = strtok(NULL, " ");
            }

            if (alreadyInRecursiveDependentPackageNamesStringBuffer) {
                continue;
            }
        }

        ////////////////////////////////////////////////////////////////

        size_t packageNameLength = strlen(packageName);

        if ((recursiveDependentPackageNamesStringBufferSize + packageNameLength + 2U) > recursiveDependentPackageNamesStringBufferCapacity) {
            char * p = (char*)realloc(recursiveDependentPackageNamesStringBuffer, (recursiveDependentPackageNamesStringBufferCapacity + 256U) * sizeof(char*));

            if (p == NULL) {
                free(recursiveDependentPackageNamesStringBuffer);
                recursiveDependentPackageNamesStringBuffer = NULL;

                free(packageNameStack);
                packageNameStack = NULL;

                return PPKG_ERROR_MEMORY_ALLOCATE;
            }

            recursiveDependentPackageNamesStringBuffer = p;
            recursiveDependentPackageNamesStringBufferCapacity += 256U;
        }

        if (recursiveDependentPackageNamesStringBufferSize == 0U) {
            strncpy(recursiveDependentPackageNamesStringBuffer, packageName, packageNameLength);
            recursiveDependentPackageNamesStringBufferSize = packageNameLength;
            recursiveDependentPackageNamesStringBuffer[recursiveDependentPackageNamesStringBufferSize] = '\0';
        } else {
            recursiveDependentPackageNamesStringBuffer[recursiveDependentPackageNamesStringBufferSize] = ' ';
            strncpy(recursiveDependentPackageNamesStringBuffer + recursiveDependentPackageNamesStringBufferSize + 1U, packageName, packageNameLength);
            recursiveDependentPackageNamesStringBufferSize += packageNameLength + 1U;
            recursiveDependentPackageNamesStringBuffer[recursiveDependentPackageNamesStringBufferSize] = '\0';
        }

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
            size_t  depPackageNamesLength = strlen(formula->dep_pkg);

            size_t  depPackageNamesCopyLength = depPackageNamesLength + 1U;
            char    depPackageNamesCopy[depPackageNamesCopyLength];
            strncpy(depPackageNamesCopy, formula->dep_pkg, depPackageNamesCopyLength);

            char * depPackageName = strtok(depPackageNamesCopy, " ");

            while (depPackageName != NULL) {
                PPKGPackage * depPackage = NULL;

                for (size_t i = 0U; i < packageSetSize; i++) {
                    if (strcmp(packageSet[i]->packageName, depPackageName) == 0) {
                        depPackage = packageSet[i];
                        break;
                    }
                }

                ////////////////////////////////////////////////////////////////

                if (packageNameStackSize == packageNameStackCapacity) {
                    char ** p = (char**)realloc(packageNameStack, (packageNameStackCapacity + 8U) * sizeof(char*));

                    if (p == NULL) {
                        free(recursiveDependentPackageNamesStringBuffer);
                        recursiveDependentPackageNamesStringBuffer = NULL;

                        free(packageNameStack);
                        packageNameStack = NULL;

                        return PPKG_ERROR_MEMORY_ALLOCATE;
                    }

                    packageNameStack = p;
                    packageNameStackCapacity += 8U;
                }

                packageNameStack[packageNameStackSize] = depPackage->packageName;
                packageNameStackSize++;

                ////////////////////////////////////////////////////////////////

                depPackageName = strtok(NULL, " ");
            }
        }
    }

    free(packageNameStack);

    (*precursiveDependentPackageNamesStringBuffer)        = recursiveDependentPackageNamesStringBuffer;
    (*precursiveDependentPackageNamesStringBufferSize)    = recursiveDependentPackageNamesStringBufferSize;
    (*precursiveDependentPackageNamesStringBufferCapacity) = recursiveDependentPackageNamesStringBufferCapacity;

    return PPKG_OK;
}

static int check_and_read_formula_in_cache(const char * packageName, const char * sessionDIR, PPKGPackage *** ppackageSet, size_t * ppackageSetSize, size_t * ppackageSetCapacity) {
    size_t         packageSetCapacity = (*ppackageSetCapacity);
    size_t         packageSetSize    = (*ppackageSetSize);
    PPKGPackage ** packageSet        = (*ppackageSet);

    size_t   packageNameStackCapacity = 10U;
    size_t   packageNameStackSize    = 0U;
    char * * packageNameStack = (char**)malloc(10 * sizeof(char*));

    if (packageNameStack == NULL) {
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    packageNameStack[0] = strdup(packageName);

    if (packageNameStack[0] == NULL) {
        free(packageNameStack);
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    packageNameStackSize = 1U;

    int ret = PPKG_OK;

    ////////////////////////////////////////////////////////////////

    while (packageNameStackSize > 0U) {
        size_t topIndex = packageNameStackSize - 1U;
        char * packageName = packageNameStack[topIndex];
        packageNameStack[topIndex] = NULL;
        packageNameStackSize--;

        PPKGFormula * formula = NULL;

        for (size_t i = 0U; i < packageSetSize; i++) {
            if (strcmp(packageSet[i]->packageName, packageName) == 0) {
                free(packageName);

                packageName = packageSet[i]->packageName;
                formula = packageSet[i]->formula;

                size_t lastIndex = packageSetSize - 1U;

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

            size_t   formulaFilePath2Length = strlen(sessionDIR) + strlen(packageName) + 6U;
            char     formulaFilePath2[formulaFilePath2Length];
            snprintf(formulaFilePath2, formulaFilePath2Length, "%s/%s.yml", sessionDIR, packageName);

            ret = ppkg_copy_file(formulaFilePath, formulaFilePath2);

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

            if (packageSetSize == packageSetCapacity) {
                PPKGPackage ** p = (PPKGPackage**)realloc(packageSet, (packageSetCapacity + 10U) * sizeof(PPKGPackage*));

                if (p == NULL) {
                    free(packageName);
                    ppkg_formula_free(formula);
                    ret = PPKG_ERROR_MEMORY_ALLOCATE;
                    goto finalize;
                }

                packageSet = p;
                packageSetCapacity += 10U;
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
        }

        if (formula->dep_pkg == NULL) {
            continue;
        }

        size_t  depPackageNamesLength = strlen(formula->dep_pkg);

        size_t  depPackageNamesCopyLength = depPackageNamesLength + 1U;
        char    depPackageNamesCopy[depPackageNamesCopyLength];
        strncpy(depPackageNamesCopy, formula->dep_pkg, depPackageNamesCopyLength);

        char * depPackageName = strtok(depPackageNamesCopy, " ");

        while (depPackageName != NULL) {
            if (strcmp(depPackageName, packageName) == 0) {
                fprintf(stderr, "package '%s' depends itself.\n", packageName);
                ret = PPKG_ERROR;
                goto finalize;
            }

            ////////////////////////////////////////////////////////////////

            if (packageNameStackSize == packageNameStackCapacity) {
                char ** p = (char**)realloc(packageNameStack, (packageNameStackCapacity + 10U) * sizeof(char*));

                if (p == NULL) {
                    ret = PPKG_ERROR_MEMORY_ALLOCATE;
                    goto finalize;
                }

                packageNameStack = p;
                packageNameStackCapacity += 10U;
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

finalize:
    for (size_t i = 0U; i < packageNameStackSize; i++) {
        free(packageNameStack[i]);
        packageNameStack[i] = NULL;
    }

    free(packageNameStack);
    packageNameStack = NULL;

    if (ret == PPKG_OK) {
        (*ppackageSet) = packageSet;
        (*ppackageSetSize) = packageSetSize;
        (*ppackageSetCapacity) = packageSetCapacity;
    } else {
        for (size_t i = 0U; i < packageSetSize; i++) {
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

static int try_compile(const char * compilerFilePath, const char * compilerOption, const char * sourceFilePath) {
    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (pid == 0) {
        execl(compilerFilePath, compilerFilePath, compilerOption, sourceFilePath, NULL);
        perror(compilerFilePath);
        exit(255);
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
            fprintf(stderr, "running command '%s %s %s' exit with status code: %d\n", compilerFilePath, compilerOption, sourceFilePath, WEXITSTATUS(childProcessExitStatusCode));
        } else if (WIFSIGNALED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '%s %s %s' killed by signal: %d\n", compilerFilePath, compilerOption, sourceFilePath, WTERMSIG(childProcessExitStatusCode));
        } else if (WIFSTOPPED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command '%s %s %s' stopped by signal: %d\n", compilerFilePath, compilerOption, sourceFilePath, WSTOPSIG(childProcessExitStatusCode));
        }

        return PPKG_ERROR;
    }
}

static int check_if_compiler_support_Wno_error_unused_command_line_argument(const char * sessionDIR, size_t sessionDIRLength, const char * compiler, bool iscc) {
    size_t testCFilePathCapacity = sessionDIRLength + 10U;
    char   testCFilePath[testCFilePathCapacity];

    int ret;

    if (iscc) {
        ret = snprintf(testCFilePath, testCFilePathCapacity, "%s/test.c",  sessionDIR);
    } else {
        ret = snprintf(testCFilePath, testCFilePathCapacity, "%s/test.cc", sessionDIR);
    }

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    int fd = open(testCFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(testCFilePath);
        return PPKG_ERROR;
    }

    const char * testCCode = "int main() {\nreturn 0;\n}\n";

    size_t testCCodeLength = strlen(testCCode);

    ssize_t written = write(fd, testCCode, testCCodeLength);

    if (written == -1) {
        perror(testCFilePath);
        close(fd);
        return PPKG_ERROR;
    }

    close(fd);

    if ((size_t)written != testCCodeLength) {
        fprintf(stderr, "file not fully written: %s\n", testCFilePath);
        return PPKG_ERROR;
    }

    return try_compile(compiler, "-Wno-error=unused-command-line-argument", testCFilePath);
}

int ppkg_toolchain_setup(
        PPKGToolChain * toolchain,
        const char * sessionDIR,
        const size_t sessionDIRLength,
        const char * ppkgCoreLibexecDIR,
        const size_t ppkgCoreLibexecDIRLength,
        const PPKGInstallOptions installOptions) {
    struct stat st;

    const size_t ccLength = strlen(toolchain->cc);

    const char * compilerNames[3] = { "cc", "c++", "objc" };

    for (int i = 0; i < 3; i++) {
        const char * compilerName = compilerNames[i];

        size_t   wrapperFilePathLength = ppkgCoreLibexecDIRLength + 14U;
        char     wrapperFilePath[wrapperFilePathLength];
        snprintf(wrapperFilePath, wrapperFilePathLength, "%s/wrapper-%s", ppkgCoreLibexecDIR, compilerName);

        if (stat(wrapperFilePath, &st) != 0) {
            size_t   outputFilePathLength = sessionDIRLength + 16U;
            char     outputFilePath[outputFilePathLength];
            snprintf(outputFilePath, outputFilePathLength, "%s/wrapper-%s", sessionDIR, compilerName);

            size_t   cmdLength = ccLength + outputFilePathLength + ppkgCoreLibexecDIRLength + 20U;
            char     cmd[cmdLength];
            snprintf(cmd, cmdLength, "%s -o %s %s/wrapper-%s.c", toolchain->cc, outputFilePath, ppkgCoreLibexecDIR, compilerName);

            int ret = run_cmd(cmd, STDOUT_FILENO);

            if (ret != PPKG_OK) {
                return ret;
            }

            if (rename(outputFilePath, wrapperFilePath) != 0) {
                perror(wrapperFilePath);
                return PPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////

    const KV kvs[3] = {
        { "PROXIED_CC",   toolchain->cc   },
        { "PROXIED_CXX",  toolchain->cxx  },
        { "PROXIED_OBJC", toolchain->objc == NULL ? toolchain->cc : toolchain->objc }
    };

    for (int i = 0; i < 3; i++) {
        const char * name  = kvs[i].name;
        const char * value = kvs[i].value;

        if (value == NULL) {
            if (unsetenv(name) != 0) {
                perror(name);
                return PPKG_ERROR;
            }
        } else {
            if (setenv(name, value, 1) != 0) {
                perror(name);
                return PPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    int ret1 = check_if_compiler_support_Wno_error_unused_command_line_argument(sessionDIR, sessionDIRLength, toolchain->cc,  true);
    int ret2 = check_if_compiler_support_Wno_error_unused_command_line_argument(sessionDIR, sessionDIRLength, toolchain->cxx, true);

    //////////////////////////////////////////////////////////////////////////////

    free(toolchain->cc);
    free(toolchain->cxx);
    free(toolchain->objc);

    //////////////////////////////////////////////////////////////////////

    char * fields[3] = {NULL};

    for (int i = 0; i < 3; i++) {
        size_t capacity = ppkgCoreLibexecDIRLength + 14U;

        char * buf = (char*)malloc(capacity);

        if (buf == NULL) {
            perror(NULL);

            for (int i = 0; i < 3; i++) {
                free(fields[i]);
            }

            return PPKG_ERROR_MEMORY_ALLOCATE;
        }

        int ret = snprintf(buf, capacity, "%s/wrapper-%s", ppkgCoreLibexecDIR, compilerNames[i]);

        if (ret < 0) {
            perror(NULL);

            for (int i = 0; i < 3; i++) {
                free(fields[i]);
            }

            return PPKG_ERROR;
        }

        fields[i] = buf;
    }

    //////////////////////////////////////////////////////////////////////

    toolchain->cc   = fields[0];
    toolchain->cxx  = fields[1];
    toolchain->objc = fields[2];

    //////////////////////////////////////////////////////////////////////

    size_t cppCapacity = strlen(toolchain->cc) + 4U;

    char * cpp = (char*)malloc(cppCapacity);

    if (cpp == NULL) {
        perror(NULL);
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    int ret = snprintf(cpp, cppCapacity, "%s -E", toolchain->cc);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    toolchain->cpp = cpp;

    //////////////////////////////////////////////////////////////////////////////

    size_t ccflagsCapacity = strlen(toolchain->ccflags) + 60U;

    char * ccflags = (char*)malloc(ccflagsCapacity);

    if (ccflags == NULL) {
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    ret = snprintf(ccflags, ccflagsCapacity, "%s%s%s%s", toolchain->ccflags, (installOptions.buildType == PPKGBuildType_release) ? " -Os" : " -g -O0", installOptions.verbose_cc ? " -v" : "", (ret1 == 0) ? " -Wno-error=unused-command-line-argument" : "");

    if (ret < 0) {
        perror(NULL);
        free(ccflags);
        return PPKG_ERROR;
    }

    free(toolchain->ccflags);

    toolchain->ccflags = ccflags;

    //////////////////////////////////////////////////////////////////////

    size_t cxxflagsCapacity = strlen(toolchain->cxxflags) + 60U;

    char * cxxflags = (char*)malloc(cxxflagsCapacity);

    if (cxxflags == NULL) {
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    ret = snprintf(cxxflags, cxxflagsCapacity, "%s%s%s%s", toolchain->cxxflags, (installOptions.buildType == PPKGBuildType_release) ? " -Os" : " -g -O0", installOptions.verbose_cc ? " -v" : "", (ret2 == 0) ? " -Wno-error=unused-command-line-argument" : "");

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    free(toolchain->cxxflags);

    toolchain->cxxflags = cxxflags;

    //////////////////////////////////////////////////////////////////////

    if (installOptions.buildType == PPKGBuildType_release || installOptions.verbose_ld) {
        size_t ldflagsCapacity = strlen(toolchain->ldflags) + 60U;

        char * ldflags = (char*)malloc(ldflagsCapacity);

        if (ldflags == NULL) {
            return PPKG_ERROR_MEMORY_ALLOCATE;
        }

        ret = snprintf(ldflags, ldflagsCapacity, "%s%s%s", toolchain->ldflags, (installOptions.buildType == PPKGBuildType_release) ? " -Wl,-s" : "", installOptions.verbose_ld ? " -Wl,-v" : "");

        if (ret < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        free(toolchain->ldflags);

        toolchain->ldflags = ldflags;
    }

    //////////////////////////////////////////////////////////////////////

    const KV envs[17] = {
        { "CC", toolchain->cc },
        { "OBJC", toolchain->objc },
        { "CXX", toolchain->cxx },
        { "CPP", toolchain->cpp },
        { "AS", toolchain->as },
        { "AR", toolchain->ar },
        { "RANLIB", toolchain->ranlib },
        { "LD", toolchain->ld },
        { "NM", toolchain->nm },
        { "SIZE", toolchain->size },
        { "STRIP", toolchain->strip },
        { "STRINGS", toolchain->strings },
        { "OBJCOPY", toolchain->objcopy },
        { "OBJDUMP", toolchain->objdump },
        { "READELF", toolchain->readelf },
        { "ADDR2LINE", toolchain->addr2line },
        { "SYSROOT", toolchain->sysroot }
    };

    for (int i = 0; i < 17; i++) {
        const char * name  = envs[i].name;
        const char * value = envs[i].value;

        if (value == NULL) {
            if (unsetenv(name) != 0) {
                perror(name);
                return PPKG_ERROR;
            }

            size_t   name2Length = strlen(name) + 11U;
            char     name2[name2Length];
            snprintf(name2, name2Length, "%s_FOR_BUILD", name);

            if (unsetenv(name2) != 0) {
                perror(name2);
                return PPKG_ERROR;
            }
        } else {
            if (setenv(name, value, 1) != 0) {
                perror(name);
                return PPKG_ERROR;
            }

            size_t   name2Length = strlen(name) + 11U;
            char     name2[name2Length];
            snprintf(name2, name2Length, "%s_FOR_BUILD", name);

            if (setenv(name2, value, 1) != 0) {
                perror(name2);
                return PPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

#if defined (__APPLE__)
    // https://keith.github.io/xcode-man-pages/xcrun.1.html
    if (setenv("SDKROOT", toolchain->sysroot, 1) != 0) {
        perror("SDKROOT");
        return PPKG_ERROR;
    }

    // https://clang.llvm.org/docs/CommandGuide/clang.html#envvar-MACOSX_DEPLOYMENT_TARGET
    // https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-mmacosx-version-min
    // If -mmacosx-version-min is unspecified, the default deployment target is read from MACOSX_DEPLOYMENT_TARGET environment variable
    if (unsetenv("MACOSX_DEPLOYMENT_TARGET") != 0) {
        perror("MACOSX_DEPLOYMENT_TARGET");
        return PPKG_ERROR;
    }
#endif

    return PPKG_OK;
}

int ppkg_install(const char * packageName, PPKGInstallOptions installOptions) {
#if defined (__APPLE__)
    if (installOptions.linkType == PPKGLinkType_static_full) {
        fprintf(stderr, "--link-type=static-full option is not supported on macOS, because there is no static standard C library on macOS. We will use --link-type=static-most instead.\n");
        installOptions.linkType = PPKGLinkType_static_most;
    }
#endif

    // redirect all stdout and stderr to /dev/null
    if (installOptions.logLevel == PPKGLogLevel_silent) {
        int fd = open("/dev/null", O_CREAT | O_TRUNC | O_WRONLY, 0666);

        if (fd < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror(NULL);
            close(fd);
            return PPKG_ERROR;
        }

        if (dup2(fd, STDERR_FILENO) < 0) {
            perror(NULL);
            close(fd);
            return PPKG_ERROR;
        }
    }

    const char * const PATH = getenv("PATH");

    if (PATH == NULL || PATH[0] == '\0') {
        return PPKG_ERROR_ENV_PATH_NOT_SET;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * PPKG_URL_TRANSFORM = getenv("PPKG_URL_TRANSFORM");

    if (PPKG_URL_TRANSFORM != NULL && PPKG_URL_TRANSFORM[0] != '\0') {
        if (setenv("UPPM_URL_TRANSFORM", PPKG_URL_TRANSFORM, 1) != 0) {
            perror("UPPM_URL_TRANSFORM");
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    // https://perldoc.perl.org/cpan#PERL_MM_USE_DEFAULT
    // Would you like to configure as much as possible automatically?
    if (setenv("PERL_MM_USE_DEFAULT", "1", 1) != 0) {
        perror("PERL_MM_USE_DEFAULT");
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * unsetenvs[13] = { "LIBS", "TARGET_ARCH", "AUTOCONF", "AUTOHEADER", "AUTOM4TE", "AUTOMAKE", "AUTOPOINT", "ACLOCAL", "GTKDOCIZE", "INTLTOOLIZE", "LIBTOOLIZE", "M4", "MAKE" };

    for (int i = 0; i < 13; i++) {
        if (unsetenv(unsetenvs[i]) != 0) {
            perror(unsetenvs[i]);
            return PPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    char   ppkgHomeDIR[PATH_MAX] = {0};
    size_t ppkgHomeDIRLength;

    int ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t ppkgCoreDIRCapacity = ppkgHomeDIRLength + 6U;
    char   ppkgCoreDIR[ppkgCoreDIRCapacity];

    ret = snprintf(ppkgCoreDIR, ppkgCoreDIRCapacity, "%s/core", ppkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    size_t ppkgCoreLibexecDIRCapacity = ppkgCoreDIRCapacity + 9U;
    char   ppkgCoreLibexecDIR[ppkgCoreLibexecDIRCapacity];

    ret = snprintf(ppkgCoreLibexecDIR, ppkgCoreLibexecDIRCapacity, "%s/libexec", ppkgCoreDIR);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    struct stat st;

    //////////////////////////////////////////////////////////////////////////////

    if (getenv("SSL_CERT_FILE") == NULL) {
        size_t   cacertFilePathCapacity = ppkgCoreDIRCapacity + 16U;
        char     cacertFilePath[cacertFilePathCapacity];
        snprintf(cacertFilePath, cacertFilePathCapacity, "%s/etc/cacert.pem", ppkgCoreDIR);

        if (stat(cacertFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            // https://www.openssl.org/docs/man1.1.1/man3/SSL_CTX_set_default_verify_paths.html
            if (setenv("SSL_CERT_FILE", cacertFilePath, 1) != 0) {
                perror("SSL_CERT_FILE");
                return PPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    char   sessionDIR[PATH_MAX];
    size_t sessionDIRLength;

    ret = ppkg_session_dir(sessionDIR, PATH_MAX, &sessionDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t   ppkgDownloadsDIRLength = ppkgHomeDIRLength + 11U;
    char     ppkgDownloadsDIR[ppkgDownloadsDIRLength];
    snprintf(ppkgDownloadsDIR, ppkgDownloadsDIRLength, "%s/downloads", ppkgHomeDIR);

    if (stat(ppkgDownloadsDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(ppkgDownloadsDIR) != 0) {
                perror(ppkgDownloadsDIR);
                return PPKG_ERROR;
            }

            if (mkdir(ppkgDownloadsDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(ppkgDownloadsDIR);
                    return PPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(ppkgDownloadsDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(ppkgDownloadsDIR);
                return PPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    char   uppmPackageInstalledRootDIR[PATH_MAX];
    size_t uppmPackageInstalledRootDIRLength;

    const char * const uppmHomeDIR = getenv("UPPM_HOME");

    if (uppmHomeDIR == NULL || uppmHomeDIR[0] == '\0') {
        const char * const userHomeDIR = getenv("HOME");

        if (userHomeDIR == NULL) {
            return PPKG_ERROR_ENV_HOME_NOT_SET;
        }

        if (userHomeDIR[0] == '\0') {
            return PPKG_ERROR_ENV_HOME_NOT_SET;
        }

        uppmPackageInstalledRootDIRLength = snprintf(uppmPackageInstalledRootDIR, PATH_MAX, "%s/.uppm/installed", userHomeDIR);
    } else {
        uppmPackageInstalledRootDIRLength = snprintf(uppmPackageInstalledRootDIR, PATH_MAX, "%s/installed", uppmHomeDIR);
    }

    if (uppmPackageInstalledRootDIRLength < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    PPKGToolChain toolchain = {0};

    ret = ppkg_toolchain_locate(&toolchain);

    if (ret != PPKG_OK) {
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = ppkg_toolchain_setup(&toolchain, sessionDIR, sessionDIRLength, ppkgCoreLibexecDIR, ppkgCoreLibexecDIRCapacity, installOptions);

    if (ret != PPKG_OK) {
        ppkg_toolchain_free(toolchain);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    SysInfo sysinfo = {0};

    ret = sysinfo_make(&sysinfo);

    if (ret != PPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    char * ppkgExeFilePath = self_realpath();

    if (ppkgExeFilePath == NULL) {
        sysinfo_free(sysinfo);
        ppkg_toolchain_free(toolchain);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t         packageSetCapacity = 0U;
    size_t         packageSetSize     = 0U;
    PPKGPackage ** packageSet         = NULL;

    ret = check_and_read_formula_in_cache(packageName, sessionDIR, &packageSet, &packageSetSize, &packageSetCapacity);

    if (ret != PPKG_OK) {
        goto finalize;
    }

    //////////////////////////////////////////////////////////////////////////////

    printf("install packages in order: ");

    for (int i = packageSetSize - 1; i >= 0; i--) {
        PPKGPackage * package = packageSet[i];
        char * packageName = package->packageName;
        printf("%s ", packageName);
    }

    printf("\n");

    //////////////////////////////////////////////////////////////////////////////

    for (int i = packageSetSize - 1; i >= 0; i--) {
        PPKGPackage * package = packageSet[i];
        char * packageName = package->packageName;

        if (!installOptions.force) {
            ret = ppkg_check_if_the_given_package_is_installed(packageName);

            if (ret == PPKG_OK) {
                fprintf(stderr, "package already has been installed : %s\n", packageName);
                continue;
            }
        }

        if (setenv("PATH", PATH, 1) != 0) {
            perror("PATH");
            goto finalize;
        }

        char * recursiveDependentPackageNamesStringBuffer         = NULL;
        size_t recursiveDependentPackageNamesStringBufferSize     = 0U;
        size_t recursiveDependentPackageNamesStringBufferCapacity = 0U;

        ret = getRecursiveDependentPackageNamesStringBuffer(packageName, packageSet, packageSetSize, &recursiveDependentPackageNamesStringBuffer, &recursiveDependentPackageNamesStringBufferSize, &recursiveDependentPackageNamesStringBufferCapacity);

        if (ret != PPKG_OK) {
            goto finalize;
        }

        //printf("%s:%zu:%s\n", packageName, recursiveDependentPackageNamesStringBufferSize, recursiveDependentPackageNamesStringBuffer);

        ret = ppkg_install_package(packageName, package->formula, installOptions, toolchain, sysinfo, uppmPackageInstalledRootDIR, uppmPackageInstalledRootDIRLength, ppkgExeFilePath, ppkgHomeDIR, ppkgHomeDIRLength, ppkgCoreDIR, ppkgCoreDIRCapacity, ppkgCoreLibexecDIR, ppkgDownloadsDIR, ppkgDownloadsDIRLength, sessionDIR, sessionDIRLength, (const char *)recursiveDependentPackageNamesStringBuffer, recursiveDependentPackageNamesStringBufferSize);

        free(recursiveDependentPackageNamesStringBuffer);
        recursiveDependentPackageNamesStringBuffer = NULL;

        if (ret != PPKG_OK) {
            goto finalize;
        }
    }

finalize:
    sysinfo_free(sysinfo);
    ppkg_toolchain_free(toolchain);
    free(ppkgExeFilePath);

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
