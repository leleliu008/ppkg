#include <stdio.h>

#include <limits.h>
#include <sys/stat.h>

#include "ppkg.h"

int ppkg_setenv_SSL_CERT_FILE() {
    if (getenv("SSL_CERT_FILE") != NULL) {
        return PPKG_OK;
    }

    int ret;

    char cacertFilePath[PATH_MAX];

    const char * const ppkgHomeDIR = getenv("PPKG_HOME");

    if (ppkgHomeDIR == NULL || ppkgHomeDIR[0] == '\0') {
        const char * const userHomeDIR = getenv("HOME");

        if (userHomeDIR == NULL || userHomeDIR[0] == '\0') {
            return PPKG_ERROR_ENV_HOME_NOT_SET;
        }

        ret = snprintf(cacertFilePath, PATH_MAX, "%s/.ppkg/core/cacert.pem", userHomeDIR);
    } else {
        ret = snprintf(cacertFilePath, PATH_MAX, "%s/core/cacert.pem", ppkgHomeDIR);
    }

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    struct stat st;

    if (stat(cacertFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
        // https://www.openssl.org/docs/man1.1.1/man3/SSL_CTX_set_default_verify_paths.html
        if (setenv("SSL_CERT_FILE", cacertFilePath, 1) != 0) {
            perror("SSL_CERT_FILE");
            return PPKG_ERROR;
        }
    }

    return PPKG_OK;
}
