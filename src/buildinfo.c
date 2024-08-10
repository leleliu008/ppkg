#include <stdio.h>

#include <zlib.h>
#include <git2.h>
#include <yaml.h>
#include <jansson.h>
#include <archive.h>
#include <curl/curlver.h>
#include <openssl/opensslv.h>

//#define PCRE2_CODE_UNIT_WIDTH 8
//#include <pcre2.h>

#include "ppkg.h"

int ppkg_buildinfo() {
    printf("ppkg.build.utctime: %s\n", PPKG_BUILD_TIMESTAMP);

    //printf("pcre2   : %d.%d\n", PCRE2_MAJOR, PCRE2_MINOR);
    printf("ppkg.build.libyaml: %s\n", yaml_get_version_string());
    printf("ppkg.build.libcurl: %s\n", LIBCURL_VERSION);
    printf("ppkg.build.libgit2: %s\n", LIBGIT2_VERSION);

//https://www.openssl.org/docs/man3.0/man3/OPENSSL_VERSION_BUILD_METADATA.html
//https://www.openssl.org/docs/man1.1.1/man3/OPENSSL_VERSION_TEXT.html
#ifdef OPENSSL_VERSION_STR
    printf("ppkg.build.openssl: %s\n", OPENSSL_VERSION_STR);
#else
    printf("ppkg.build.openssl: %s\n", OPENSSL_VERSION_TEXT);
#endif

    printf("ppkg.build.jansson: %s\n", JANSSON_VERSION);
    printf("ppkg.build.archive: %s\n", ARCHIVE_VERSION_ONLY_STRING);
    printf("ppkg.build.zlib:    %s\n", ZLIB_VERSION);

    return PPKG_OK;
}
