#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

int main(int argc, char* argv[]) {
    char buf[PATH_MAX];

    int ret = readlink("/proc/self/exe", buf, PATH_MAX);

    if (ret == -1) {
        perror("/proc/self/exe");
        return 1;
    }

    ////////////////////////////////////////////////////

    int slashIndex = -1;

    char realExePath[ret + 5];

    for (int i = 0; i < ret; i++) {
        realExePath[i] = buf[i];

        if (buf[i] == '/') {
            slashIndex = i;
        }
    }

    realExePath[ret    ] = '.';
    realExePath[ret + 1] = 'e';
    realExePath[ret + 2] = 'x';
    realExePath[ret + 3] = 'e';
    realExePath[ret + 4] = '\0';

    ////////////////////////////////////////////////////

    const char * relativePath = "/../.ppkg/dependencies/lib";
    const char * dynamicLoaderName = "ld-linux-x86-64.so.2";

    ////////////////////////////////////////////////////

    for (int i = 1; ; i++) {
        buf[slashIndex + i] = relativePath[i];

        if (relativePath[i] == '\0') {
            break;
        }
    }

    ////////////////////////////////////////////////////

    char dynamicLoaderPath[PATH_MAX];

    ret = snprintf(dynamicLoaderPath, PATH_MAX, "%s/%s", buf, dynamicLoaderName);

    if (ret < 0) {
        perror(NULL);
        return 2;
    }

    ////////////////////////////////////////////////////

    char* argv2[argc + 4];

    argv2[0] = dynamicLoaderPath;
    argv2[1] = (char*)"--library-path";
    argv2[2] = buf;
    argv2[3] = realExePath;

    for (int i = 1; i < argc; i++) {
        argv2[i + 3] = argv[i];
    }

    argv2[argc + 3] = NULL;

    execv (dynamicLoaderPath, argv2);
    perror(dynamicLoaderPath);
    return 255;
}
