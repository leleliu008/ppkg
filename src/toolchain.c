#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "core/exe.h"

#include "ppkg.h"

static int read_from_fd(int inputFD, char ** outP) {
    // PATH_MAX : maximum number of bytes in a pathname, including the terminating null character.
    // https://pubs.opengroup.org/onlinepubs/009695399/basedefs/limits.h.html
    char buf[PATH_MAX];

    ssize_t readSize = read(inputFD, buf, PATH_MAX - 1U);

    if (readSize < 0) {
        return -1;
    }

    if (readSize == 0) {
        return 0;
    }

    if (buf[readSize - 1] == '\n') {
        readSize--;
    }

    if (readSize > 0) {
        buf[readSize] = '\0';

        char * p = strdup(buf);

        if (p == NULL) {
            return -1;
        }

        (*outP) = p;
    }

    return 0;
}

// https://keith.github.io/xcode-man-pages/xcrun.1.html
static int xcrun_show_sdk_path(char ** outP) {
    int pipeFDs[2];

    if (pipe(pipeFDs) != 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (pid == 0) {
        close(pipeFDs[0]);

        if (dup2(pipeFDs[1], STDOUT_FILENO) < 0) {
            perror(NULL);
            exit(254);
        }

        execl("xcrun", "xcrun", "--sdk", "macosx", "--show-sdk-path", NULL);

        perror("xcrun");

        exit(255);
    } else {
        close(pipeFDs[1]);

        int ret = read_from_fd(pipeFDs[0], outP);

        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (childProcessExitStatusCode == 0) {
            if (ret == 0) {
                return PPKG_OK;
            } else {
                perror(NULL);
                return PPKG_ERROR;
            }
        }

        if (WIFEXITED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command 'xcrun --sdk macosx --show-sdk-path' exit with status code: %d\n", WEXITSTATUS(childProcessExitStatusCode));
        } else if (WIFSIGNALED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command 'xcrun --sdk macosx --show-sdk-path' killed by signal: %d\n", WTERMSIG(childProcessExitStatusCode));
        } else if (WIFSTOPPED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command 'xcrun --sdk macosx --show-sdk-path' stopped by signal: %d\n", WSTOPSIG(childProcessExitStatusCode));
        }

        return PPKG_ERROR;
    }
}

// https://keith.github.io/xcode-man-pages/xcrun.1.html
static int xcrun_find(const char * what, char ** outP) {
    int pipeFDs[2];

    if (pipe(pipeFDs) != 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (pid == 0) {
        close(pipeFDs[0]);

        if (dup2(pipeFDs[1], STDOUT_FILENO) < 0) {
            perror(NULL);
            exit(254);
        }

        execl("xcrun", "xcrun", "--sdk", "macosx", "--find", what, NULL);

        perror("xcrun");

        exit(255);
    } else {
        close(pipeFDs[1]);

        int ret = read_from_fd(pipeFDs[0], outP);

        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (childProcessExitStatusCode == 0) {
            if (ret == 0) {
                return PPKG_OK;
            } else {
                perror(NULL);
                return PPKG_ERROR;
            }
        }

        if (WIFEXITED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command 'xcrun --sdk macosx --find %s' exit with status code: %d\n", what, WEXITSTATUS(childProcessExitStatusCode));
        } else if (WIFSIGNALED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command 'xcrun --sdk macosx --find %s' killed by signal: %d\n", what, WTERMSIG(childProcessExitStatusCode));
        } else if (WIFSTOPPED(childProcessExitStatusCode)) {
            fprintf(stderr, "running command 'xcrun --sdk macosx --find %s' stopped by signal: %d\n", what, WSTOPSIG(childProcessExitStatusCode));
        }

        return PPKG_ERROR;
    }
}

static int ppkg_toolchain_find(PPKGToolChain * toolchain) {
    char * cc = NULL;

    int ret = exe_lookup("cc", &cc, NULL);

    if (ret == PPKG_OK) {
        if (cc == NULL) {
            fprintf(stderr, "C Compiler Not Found.\n");
            return PPKG_ERROR;
        } else {
            toolchain->cc = cc;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * cxx = NULL;

    ret = exe_lookup("c++", &cxx, NULL);

    if (ret == PPKG_OK) {
        if (cc == NULL) {
            fprintf(stderr, "C++ Compiler Not Found.\n");
            return PPKG_ERROR;
        } else {
            toolchain->cxx = cxx;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * as = NULL;

    ret = exe_lookup("as", &as, NULL);

    if (ret == PPKG_OK) {
        if (as == NULL) {
            fprintf(stderr, "command not found: as\n");
            return PPKG_ERROR;
        } else {
            toolchain->as = as;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * ar = NULL;

    ret = exe_lookup("ar", &ar, NULL);

    if (ret == PPKG_OK) {
        if (ar == NULL) {
            fprintf(stderr, "command not found: ar\n");
            return PPKG_ERROR;
        } else {
            toolchain->ar = ar;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * ranlib = NULL;

    ret = exe_lookup("ranlib", &ranlib, NULL);

    if (ret == PPKG_OK) {
        if (ranlib == NULL) {
            fprintf(stderr, "command not found: ranlib\n");
            return PPKG_ERROR;
        } else {
            toolchain->ranlib = ranlib;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * ld = NULL;

    ret = exe_lookup("ld", &ld, NULL);

    if (ret == PPKG_OK) {
        if (ld == NULL) {
            fprintf(stderr, "command not found: ld\n");
            return PPKG_ERROR;
        } else {
            toolchain->ld = ld;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * nm = NULL;

    ret = exe_lookup("nm", &nm, NULL);

    if (ret == PPKG_OK) {
        if (nm == NULL) {
            fprintf(stderr, "command not found: nm\n");
            return PPKG_ERROR;
        } else {
            toolchain->nm = nm;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * size = NULL;

    ret = exe_lookup("size", &size, NULL);

    if (ret == PPKG_OK) {
        if (size == NULL) {
            fprintf(stderr, "command not found: size\n");
            return PPKG_ERROR;
        } else {
            toolchain->size = size;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * strip = NULL;

    ret = exe_lookup("strip", &strip, NULL);

    if (ret == PPKG_OK) {
        if (strip == NULL) {
            fprintf(stderr, "command not found: strip\n");
            return PPKG_ERROR;
        } else {
            toolchain->strip = strip;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * strings = NULL;

    ret = exe_lookup("strings", &strings, NULL);

    if (ret == PPKG_OK) {
        if (strings == NULL) {
            fprintf(stderr, "command not found: strings\n");
            return PPKG_ERROR;
        } else {
            toolchain->strings = strings;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * objcopy = NULL;

    ret = exe_lookup("objcopy", &objcopy, NULL);

    if (ret == PPKG_OK) {
        if (objcopy == NULL) {
            fprintf(stderr, "command not found: objcopy\n");
            return PPKG_ERROR;
        } else {
            toolchain->objcopy = objcopy;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * objdump = NULL;

    ret = exe_lookup("objdump", &objdump, NULL);

    if (ret == PPKG_OK) {
        if (objdump == NULL) {
            fprintf(stderr, "command not found: objdump\n");
            return PPKG_ERROR;
        } else {
            toolchain->objdump = objdump;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * readelf = NULL;

    ret = exe_lookup("readelf", &readelf, NULL);

    if (ret == PPKG_OK) {
        if (readelf == NULL) {
            fprintf(stderr, "command not found: readelf\n");
            return PPKG_ERROR;
        } else {
            toolchain->readelf = readelf;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * addr2line = NULL;

    ret = exe_lookup("addr2line", &addr2line, NULL);

    if (ret == PPKG_OK) {
        if (addr2line == NULL) {
            fprintf(stderr, "command not found: addr2line\n");
            return PPKG_ERROR;
        } else {
            toolchain->addr2line = addr2line;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    toolchain->cxxflags = strdup("-fPIC -fno-common");

    if (toolchain->cxxflags == NULL) {
        ppkg_toolchain_free(*toolchain);
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    toolchain->ccflags = strdup("-fPIC -fno-common");

    if (toolchain->ccflags == NULL) {
        ppkg_toolchain_free(*toolchain);
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    // https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html
    toolchain->ldflags = strdup("-Wl,--as-needed -Wl,-z,muldefs -Wl,--allow-multiple-definition");

    if (toolchain->ldflags == NULL) {
        ppkg_toolchain_free(*toolchain);
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    return PPKG_OK;
}

static int ppkg_toolchain_macos(PPKGToolChain * toolchain) {
    char * cc = NULL;

    int ret = xcrun_find("clang", &cc);

    if (ret == PPKG_OK) {
        if (cc == NULL) {
            fprintf(stderr, "C Compiler Not Found.\n");
            return PPKG_ERROR;
        } else {
            toolchain->cc = cc;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * cxx = NULL;

    ret = xcrun_find("clang++", &cxx);

    if (ret == PPKG_OK) {
        if (cc == NULL) {
            fprintf(stderr, "C++ Compiler Not Found.\n");
            return PPKG_ERROR;
        } else {
            toolchain->cxx = cxx;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * as = NULL;

    ret = xcrun_find("as", &as);

    if (ret == PPKG_OK) {
        if (as == NULL) {
            fprintf(stderr, "command not found: as\n");
            return PPKG_ERROR;
        } else {
            toolchain->as = as;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * ar = NULL;

    ret = xcrun_find("ar", &ar);

    if (ret == PPKG_OK) {
        if (ar == NULL) {
            fprintf(stderr, "command not found: ar\n");
            return PPKG_ERROR;
        } else {
            toolchain->ar = ar;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * ranlib = NULL;

    ret = xcrun_find("ranlib", &ranlib);

    if (ret == PPKG_OK) {
        if (ranlib == NULL) {
            fprintf(stderr, "command not found: ranlib\n");
            return PPKG_ERROR;
        } else {
            toolchain->ranlib = ranlib;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * ld = NULL;

    ret = xcrun_find("ld", &ld);

    if (ret == PPKG_OK) {
        if (ld == NULL) {
            fprintf(stderr, "command not found: ld\n");
            return PPKG_ERROR;
        } else {
            toolchain->ld = ld;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * nm = NULL;

    ret = xcrun_find("nm", &nm);

    if (ret == PPKG_OK) {
        if (nm == NULL) {
            fprintf(stderr, "command not found: nm\n");
            return PPKG_ERROR;
        } else {
            toolchain->nm = nm;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * size = NULL;

    ret = xcrun_find("size", &size);

    if (ret == PPKG_OK) {
        if (size == NULL) {
            fprintf(stderr, "command not found: size\n");
            return PPKG_ERROR;
        } else {
            toolchain->size = size;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * strip = NULL;

    ret = xcrun_find("strip", &strip);

    if (ret == PPKG_OK) {
        if (strip == NULL) {
            fprintf(stderr, "command not found: strip\n");
            return PPKG_ERROR;
        } else {
            toolchain->strip = strip;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * strings = NULL;

    ret = xcrun_find("strings", &strings);

    if (ret == PPKG_OK) {
        if (strings == NULL) {
            fprintf(stderr, "command not found: strings\n");
            return PPKG_ERROR;
        } else {
            toolchain->strings = strings;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * objdump = NULL;

    ret = xcrun_find("objdump", &objdump);

    if (ret == PPKG_OK) {
        if (objdump == NULL) {
            fprintf(stderr, "command not found: objdump\n");
            return PPKG_ERROR;
        } else {
            toolchain->objdump = objdump;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    //////////////////////////////////////////////

    char * sysroot = NULL;

    ret = xcrun_show_sdk_path(&sysroot);

    if (ret == PPKG_OK) {
        if (sysroot == NULL) {
            fprintf(stderr, "Can not locate MacOSX sdk path.\n");
            return PPKG_ERROR;
        } else {
            toolchain->sysroot = sysroot;
        }
    } else {
        ppkg_toolchain_free(*toolchain);
        return ret;
    }

    size_t sysrootLength = strlen(sysroot);

    size_t cppflagsCapacity = sysrootLength + 30U;
    char   cppflags[cppflagsCapacity];

    ret = snprintf(cppflags, cppflagsCapacity, "-isysroot %s -Qunused-arguments", sysroot);

    if (ret < 0) {
        ppkg_toolchain_free(*toolchain);
        return PPKG_ERROR;
    }

    size_t cxxflagsCapacity = sysrootLength + 48U;
    char   cxxflags[cxxflagsCapacity];

    ret = snprintf(cxxflags, cxxflagsCapacity, "-isysroot %s -Qunused-arguments -fPIC -fno-common", sysroot);

    if (ret < 0) {
        ppkg_toolchain_free(*toolchain);
        return PPKG_ERROR;
    }

    size_t ccflagsCapacity = sysrootLength + 48U;
    char   ccflags[ccflagsCapacity];

    ret = snprintf(ccflags, ccflagsCapacity, "-isysroot %s -Qunused-arguments -fPIC -fno-common", sysroot);

    if (ret < 0) {
        ppkg_toolchain_free(*toolchain);
        return PPKG_ERROR;
    }

    size_t ldflagsCapacity = sysrootLength + 35U;
    char   ldflags[ldflagsCapacity];

    ret = snprintf(ldflags, ldflagsCapacity, "-isysroot %s -Wl,-search_paths_first", sysroot);

    if (ret < 0) {
        ppkg_toolchain_free(*toolchain);
        return PPKG_ERROR;
    }

    toolchain->cppflags = strdup(cppflags);

    if (toolchain->cppflags == NULL) {
        ppkg_toolchain_free(*toolchain);
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    toolchain->cxxflags = strdup(cxxflags);

    if (toolchain->cxxflags == NULL) {
        ppkg_toolchain_free(*toolchain);
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    toolchain->ccflags = strdup(ccflags);

    if (toolchain->ccflags == NULL) {
        ppkg_toolchain_free(*toolchain);
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    toolchain->ldflags = strdup(ldflags);

    if (toolchain->ldflags == NULL) {
        ppkg_toolchain_free(*toolchain);
        return PPKG_ERROR_MEMORY_ALLOCATE;
    }

    return PPKG_OK;
}

int ppkg_toolchain_locate(PPKGToolChain * toolchain) {
#if defined (__APPLE__)
    return ppkg_toolchain_macos(toolchain);
#else
    return ppkg_toolchain_find(toolchain);
#endif
}

void ppkg_toolchain_dump(PPKGToolChain toolchain) {
    printf("cc:        %s\n", toolchain.cc);
    printf("objc:      %s\n", toolchain.objc);
    printf("cxx:       %s\n", toolchain.cxx);
    printf("cpp:       %s\n", toolchain.cpp);
    printf("as:        %s\n", toolchain.as);
    printf("ar:        %s\n", toolchain.ar);
    printf("ranlib:    %s\n", toolchain.ranlib);
    printf("ld:        %s\n", toolchain.ld);
    printf("nm:        %s\n", toolchain.nm);
    printf("size:      %s\n", toolchain.size);
    printf("strip:     %s\n", toolchain.strip);
    printf("strings:   %s\n", toolchain.strings);
    printf("objcopy:   %s\n", toolchain.objcopy);
    printf("objdump:   %s\n", toolchain.objdump);
    printf("readelf:   %s\n", toolchain.readelf);
    printf("dlltool:   %s\n", toolchain.dlltool);
    printf("addr2line: %s\n", toolchain.addr2line);
    printf("sysroot:   %s\n", toolchain.sysroot);
    printf("ccflags:   %s\n", toolchain.ccflags);
    printf("cxxflags:  %s\n", toolchain.cxxflags);
    printf("cppflags:  %s\n", toolchain.cppflags);
    printf("ldflags:   %s\n", toolchain.ldflags);
}

void ppkg_toolchain_free(PPKGToolChain toolchain) {
    if (toolchain.cc != NULL) {
        free(toolchain.cc);
        toolchain.cc = NULL;
    }

    if (toolchain.cxx != NULL) {
        free(toolchain.cxx);
        toolchain.cxx = NULL;
    }

    if (toolchain.objc != NULL) {
        free(toolchain.objc);
        toolchain.objc = NULL;
    }

    if (toolchain.cpp != NULL) {
        free(toolchain.cpp);
        toolchain.cpp = NULL;
    }

    if (toolchain.as != NULL) {
        free(toolchain.as);
        toolchain.as = NULL;
    }

    if (toolchain.ar != NULL) {
        free(toolchain.ar);
        toolchain.ar = NULL;
    }

    if (toolchain.ranlib != NULL) {
        free(toolchain.ranlib);
        toolchain.ranlib = NULL;
    }

    if (toolchain.ld != NULL) {
        free(toolchain.ld);
        toolchain.ld = NULL;
    }

    if (toolchain.nm != NULL) {
        free(toolchain.nm);
        toolchain.nm = NULL;
    }

    if (toolchain.size != NULL) {
        free(toolchain.size);
        toolchain.size = NULL;
    }

    if (toolchain.strip != NULL) {
        free(toolchain.strip);
        toolchain.strip = NULL;
    }

    if (toolchain.strings != NULL) {
        free(toolchain.strings);
        toolchain.strings = NULL;
    }

    if (toolchain.objcopy != NULL) {
        free(toolchain.objcopy);
        toolchain.objcopy = NULL;
    }

    if (toolchain.objdump != NULL) {
        free(toolchain.objdump);
        toolchain.objdump = NULL;
    }

    if (toolchain.readelf != NULL) {
        free(toolchain.readelf);
        toolchain.readelf = NULL;
    }

    if (toolchain.dlltool != NULL) {
        free(toolchain.dlltool);
        toolchain.dlltool = NULL;
    }

    if (toolchain.addr2line != NULL) {
        free(toolchain.addr2line);
        toolchain.addr2line = NULL;
    }

    if (toolchain.sysroot != NULL) {
        free(toolchain.sysroot);
        toolchain.sysroot = NULL;
    }

    if (toolchain.ccflags != NULL) {
        free(toolchain.ccflags);
        toolchain.ccflags = NULL;
    }

    if (toolchain.cxxflags != NULL) {
        free(toolchain.cxxflags);
        toolchain.cxxflags = NULL;
    }

    if (toolchain.cppflags != NULL) {
        free(toolchain.cppflags);
        toolchain.cppflags = NULL;
    }

    if (toolchain.ldflags != NULL) {
        free(toolchain.ldflags);
        toolchain.ldflags = NULL;
    }
}