#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <linux/limits.h>

int main(int argc, char* argv[]) {
    char selfExePath[PATH_MAX];

    int n = readlink("/proc/self/exe", selfExePath, PATH_MAX);

    if (n == -1) {
        perror("/proc/self/exe");
        return 1;
    }

    selfExePath[n] = '\0';

    ////////////////////////////////////////////////////

    int slashIndex = -1;

    char realExePath[n + 5];

    for (int i = 0; i < n; i++) {
        realExePath[i] = selfExePath[i];

        if (selfExePath[i] == '/') {
            slashIndex = i;
        }
    }

    realExePath[n    ] = '.';
    realExePath[n + 1] = 'e';
    realExePath[n + 2] = 'x';
    realExePath[n + 3] = 'e';
    realExePath[n + 4] = '\0';

    ////////////////////////////////////////////////////

    const char * dynamicLoaderName = "ld-linux-x86-64.so.2";
    const char * libraryPathRelativeToSelfExePath = "/../.ppkg/dependencies/lib";

    ////////////////////////////////////////////////////

    char libraryPath[PATH_MAX];

    for (int i = 0; i <= slashIndex; i++) {
        libraryPath[i] = selfExePath[i];
    }

    for (int i = 1; ; i++) {
        libraryPath[slashIndex + i] = libraryPathRelativeToSelfExePath[i];

        if (libraryPathRelativeToSelfExePath[i] == '\0') {
            break;
        }
    }

    ////////////////////////////////////////////////////

    char dynamicLoaderPath[PATH_MAX];

    int ret = snprintf(dynamicLoaderPath, PATH_MAX, "%s/%s", libraryPath, dynamicLoaderName);

    if (ret < 0) {
        perror(NULL);
        return 2;
    }

    ////////////////////////////////////////////////////

    char* args[argc + 6];

    args[0] = dynamicLoaderPath;
    args[1] = (char*)"--library-path";
    args[2] = libraryPath;
    args[3] = (char*)"--argv0";
    args[4] = selfExePath;
    args[5] = realExePath;

    for (int i = 1; i < argc; i++) {
        args[i + 5] = argv[i];
    }

    args[argc + 5] = NULL;

    execv (dynamicLoaderPath, args);
    perror(dynamicLoaderPath);
    return 255;
}
