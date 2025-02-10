#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <sys/stat.h>

#if defined (__linux__)
#include <linux/limits.h>
#else
#include <limits.h>
#endif

int main(int argc, char* argv[]) {
    // https://docs.appimage.org/packaging-guide/environment-variables.html#type-2-appimage-runtime
    const char * const APPDIR = getenv("APPDIR");

    if (APPDIR == NULL || APPDIR[0] == '\0') {
        fprintf(stderr, "APPDIR environment variable is not set.\n");
        return 1;
    }

    ////////////////////////////////////////////////////

    const char * const ARGV0 = getenv("ARGV0");

    if (ARGV0 == NULL || ARGV0[0] == '\0') {
        fprintf(stderr, "ARGV0 environment variable is not set.\n");
        return 2;
    }

    ////////////////////////////////////////////////////

    const char * cmdName = NULL;

    ////////////////////////////////////////////////////

    int slashIndex = -1;

    for (int i = 0; ; i++) {
        if (ARGV0[i] == '\0') {
            break;
        }

        if (ARGV0[i] == '/') {
            slashIndex = i;
        }
    }

    if (slashIndex == -1) {
        cmdName = ARGV0;
    } else {
        cmdName = &ARGV0[slashIndex + 1];
    }

    ////////////////////////////////////////////////////

    char cmdPATH[PATH_MAX];

    int APPDIRLength = 0;

    for (int i = 0; ;i++) {
        if (APPDIR[i] == '\0') {
            APPDIRLength = i;
            cmdPATH[i] = '/';
            break;
        } else {
            cmdPATH[i] = APPDIR[i];
        }
    }

    ////////////////////////////////////////////////////

    cmdPATH[APPDIRLength + 1] = 'b';
    cmdPATH[APPDIRLength + 2] = 'i';
    cmdPATH[APPDIRLength + 3] = 'n';
    cmdPATH[APPDIRLength + 4] = '/';

    char * p = &cmdPATH[APPDIRLength + 5];

    for (int i = 0; ;i++) {
        p[i] = cmdName[i];

        if (cmdName[i] == '\0') {
            break;
        }
    }

    struct stat st;

    if (stat(cmdPATH, &st) == 0 && S_ISREG(st.st_mode)) {

    } else {
        cmdPATH[APPDIRLength + 1] = 's';
        cmdPATH[APPDIRLength + 2] = 'b';
        cmdPATH[APPDIRLength + 3] = 'i';
        cmdPATH[APPDIRLength + 4] = 'n';
        cmdPATH[APPDIRLength + 5] = '/';

        p = &cmdPATH[APPDIRLength + 6];

        for (int i = 0; ;i++) {
            p[i] = cmdName[i];

            if (cmdName[i] == '\0') {
                break;
            }
        }

        if (stat(cmdPATH, &st) == 0 && S_ISREG(st.st_mode)) {

        } else {
            const char * const defaultEXE = "$APPEXEC_FILEPATH";

            p = &cmdPATH[APPDIRLength + 1];

            for (int i = 0; ;i++) {
                p[i] = defaultEXE[i];

                if (defaultEXE[i] == '\0') {
                    break;
                }
            }
        }
    }

    ////////////////////////////////////////////////////

    argv[0] = cmdPATH;
    execv (cmdPATH, argv);
    perror(cmdPATH);
    return 255;
}
