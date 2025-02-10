#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#if defined (__linux__)
#include <linux/limits.h>
#else
#include <limits.h>
#endif

int main(int argc, char* argv[]) {
    char selfExecPath[PATH_MAX];

    int ret = readlink("/proc/self/exe", selfExecPath, PATH_MAX);

    if (ret == -1) {
        perror("/proc/self/exe");
        return 1;
    }

    selfExecPath[ret] = '\0';

    ////////////////////////////////////////////////////

    int slashIndex = -1;

    char realExePath[ret + 5];

    for (int i = 0; i < ret; i++) {
        realExePath[i] = selfExecPath[i];

        if (selfExecPath[i] == '/') {
            slashIndex = i;
        }
    }

    realExePath[ret    ] = '.';
    realExePath[ret + 1] = 'e';
    realExePath[ret + 2] = 'x';
    realExePath[ret + 3] = 'e';
    realExePath[ret + 4] = '\0';

    ////////////////////////////////////////////////////

    const char * dynamicLoaderName = "ld-linux-x86-64.so.2";
    const char * libraryPathRelativeToSelfExePath = "/../.ppkg/dependencies/lib";

    ////////////////////////////////////////////////////

    char libraryPath[PATH_MAX];

    for (int i = 0; i <= slashIndex; i++) {
        libraryPath[i] = selfExecPath[i];
    }

    for (int i = 1; ; i++) {
        libraryPath[slashIndex + i] = libraryPathRelativeToSelfExePath[i];

        if (libraryPathRelativeToSelfExePath[i] == '\0') {
            break;
        }
    }

    ////////////////////////////////////////////////////

    char dynamicLoaderPath[PATH_MAX];

    ret = snprintf(dynamicLoaderPath, PATH_MAX, "%s/%s", libraryPath, dynamicLoaderName);

    if (ret < 0) {
        perror(NULL);
        return 2;
    }

    ////////////////////////////////////////////////////

    char* argv2[argc + 6];

    argv2[0] = dynamicLoaderPath;
    argv2[1] = (char*)"--library-path";
    argv2[2] = libraryPath;
    argv2[3] = (char*)"--argv0";
    argv2[4] = selfExecPath;
    argv2[5] = realExePath;

    for (int i = 1; i < argc; i++) {
        argv2[i + 5] = argv[i];
    }

    argv2[argc + 5] = NULL;

    execv (dynamicLoaderPath, argv2);
    perror(dynamicLoaderPath);
    return 255;
}
