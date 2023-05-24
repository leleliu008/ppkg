#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <openssl/evp.h>

#include "core/zlib-flate.h"
#include "core/sha256sum.h"
#include "core/base16.h"
#include "core/base64.h"
#include "core/exe.h"
#include "core/log.h"

#include "ppkg.h"

//invoked as 'ppkg util <CMD> [ARGUMENTS]'
int ppkg_util(int argc, char* argv[]) {
    if (argv[2] == NULL) {
        fprintf(stderr, "Usage: %s %s <COMMAND> , <COMMAND> is not given.\n", argv[0], argv[1]);
        return PPKG_ERROR_ARG_IS_NULL;
    }

    if (strcmp(argv[2], "base16-encode") == 0) {
        if (argv[3] == NULL) {
            unsigned char inputBuf[1024];

            for (;;) {
                size_t readSizeInBytes = fread(inputBuf, 1, 1024, stdin);

                if (ferror(stdin)) {
                    return PPKG_ERROR;
                }

                if (readSizeInBytes > 0) {
                    size_t outputBufSizeInBytes = readSizeInBytes << 1;
                    char   outputBuf[outputBufSizeInBytes];

                    if (base16_encode(outputBuf, inputBuf, readSizeInBytes, true) != 0) {
                        perror(NULL);
                        return PPKG_ERROR;
                    }

                    if (fwrite(outputBuf, 1, outputBufSizeInBytes, stdout) != outputBufSizeInBytes || ferror(stdout)) {
                        return PPKG_ERROR;
                    }
                }

                if (feof(stdin)) {
                    if (isatty(STDOUT_FILENO)) {
                        printf("\n");
                    }

                    return PPKG_OK;
                }
            }
        } else {
            unsigned char * inputBuf = (unsigned char *)argv[3];
            size_t          inputBufSizeInBytes = strlen(argv[3]);

            if (inputBufSizeInBytes == 0) {
                fprintf(stderr, "Usage: %s %s %s <STR> , <STR> should be non-empty.\n", argv[0], argv[1], argv[2]);
                return PPKG_ERROR_ARG_IS_NULL;
            }

            size_t outputBufSizeInBytes = inputBufSizeInBytes << 1;
            char   outputBuf[outputBufSizeInBytes];

            if (base16_encode(outputBuf, inputBuf, inputBufSizeInBytes, true) != 0) {
                return PPKG_ERROR;
            }

            if (fwrite(outputBuf, 1, outputBufSizeInBytes, stdout) != outputBufSizeInBytes || ferror(stdout)) {
                return PPKG_ERROR;
            }

            if (isatty(STDOUT_FILENO)) {
                printf("\n");
            }

            return PPKG_OK;
        }
    }

    if (strcmp(argv[2], "base16-decode") == 0) {
        if (argv[3] == NULL) {
            fprintf(stderr, "Usage: %s %s %s <BASE16-DECODED-STR> , <BASE16-DECODED-STR> is not given.\n", argv[0], argv[1], argv[2]);
            return PPKG_ERROR_ARG_IS_NULL;
        }

        size_t inputBufSizeInBytes = strlen(argv[3]);

        if (inputBufSizeInBytes == 0) {
            fprintf(stderr, "Usage: %s %s %s <BASE16-DECODED-STR> , <BASE16-DECODED-STR> should be non-empty.\n", argv[0], argv[1], argv[2]);
            return PPKG_ERROR_ARG_IS_NULL;
        }

        if ((inputBufSizeInBytes & 1) != 0) {
            fprintf(stderr, "Usage: %s %s %s <BASE16-DECODED-STR> , <BASE16-DECODED-STR> length should be an even number.\n", argv[0], argv[1], argv[2]);
            return PPKG_ERROR_ARG_IS_INVALID;
        }

        size_t        outputBufSizeInBytes = inputBufSizeInBytes >> 1;
        unsigned char outputBuf[outputBufSizeInBytes];

        if (base16_decode(outputBuf, argv[3], inputBufSizeInBytes) == 0) {
            if (fwrite(outputBuf, 1, outputBufSizeInBytes, stdout) != outputBufSizeInBytes || ferror(stdout)) {
                return PPKG_ERROR;
            }

            if (isatty(STDOUT_FILENO)) {
                printf("\n");
            }

            return PPKG_OK;
        } else {
            perror(NULL);

            if (errno == EINVAL) {
                return PPKG_ERROR_ARG_IS_INVALID;
            } else {
                return PPKG_ERROR;
            }
        }
    }

    if (strcmp(argv[2], "base64-encode") == 0) {
        if (argv[3] == NULL) {
            unsigned char inputBuf[1023];

            for (;;) {
                size_t readSizeInBytes = fread(inputBuf, 1, 1023, stdin);

                if (ferror(stdin)) {
                    return PPKG_ERROR;
                }

                if (readSizeInBytes > 0) {
                    unsigned int  x = (readSizeInBytes % 3) == 0 ? 0 : 1;
                    unsigned int  outputBufSizeInBytes = (readSizeInBytes / 3 + x) << 2;
                    unsigned char outputBuf[outputBufSizeInBytes];

                    int ret = EVP_EncodeBlock(outputBuf, inputBuf, readSizeInBytes);

                    if (ret < 0) {
                        return ret;
                    }

                    if (fwrite(outputBuf, 1, outputBufSizeInBytes, stdout) != outputBufSizeInBytes || ferror(stdout)) {
                        return PPKG_ERROR;
                    }
                }

                if (feof(stdin)) {
                    if (isatty(STDOUT_FILENO)) {
                        printf("\n");
                    }

                    return PPKG_OK;
                }
            }
        } else {
            unsigned char * inputBuf = (unsigned char *)argv[3];
            unsigned int    inputBufSizeInBytes = strlen(argv[3]);

            if (inputBufSizeInBytes == 0) {
                fprintf(stderr, "Usage: %s %s %s <STR> , <STR> should be non-empty.\n", argv[0], argv[1], argv[2]);
                return PPKG_ERROR_ARG_IS_NULL;
            }

            unsigned int  x = (inputBufSizeInBytes % 3) == 0 ? 0 : 1;
            unsigned int  outputBufSizeInBytes = (inputBufSizeInBytes / 3 + x) << 2;
            unsigned char outputBuf[outputBufSizeInBytes];

            int ret = EVP_EncodeBlock(outputBuf, inputBuf, inputBufSizeInBytes);

            if (ret < 0) {
                return ret;
            }

            if (fwrite(outputBuf, 1, outputBufSizeInBytes, stdout) != outputBufSizeInBytes || ferror(stdout)) {
                return PPKG_ERROR;
            }

            if (isatty(STDOUT_FILENO)) {
                printf("\n");
            }

            return PPKG_OK;
        }
    }

    if (strcmp(argv[2], "base64-decode") == 0) {
        if (argv[3] == NULL) {
            unsigned char inputBuf[1024];

            for (;;) {
                size_t readSizeInBytes = fread(inputBuf, 1, 1024, stdin);

                if (ferror(stdin)) {
                    return PPKG_ERROR;
                }

                if (readSizeInBytes > 0) {
                    unsigned int  outputBufSizeInBytes = (readSizeInBytes >> 2) * 3;
                    unsigned char outputBuf[outputBufSizeInBytes];

                    // EVP_DecodeBlock() returns the length of the data decoded or -1 on error.
                    int n = EVP_DecodeBlock(outputBuf, inputBuf, readSizeInBytes);

                    if (n < 0) {
                        return PPKG_ERROR_ARG_IS_INVALID;
                    }

                    if (fwrite(outputBuf, 1, n, stdout) != (size_t)n || ferror(stdout)) {
                        return PPKG_ERROR;
                    }
                }

                if (feof(stdin)) {
                    if (isatty(STDOUT_FILENO)) {
                        printf("\n");
                    }

                    return PPKG_OK;
                }
            }
        } else {
            unsigned char * inputBuf = (unsigned char *)argv[3];
            unsigned int    inputBufSizeInBytes = strlen(argv[3]);

            if (inputBufSizeInBytes == 0) {
                fprintf(stderr, "Usage: %s %s %s <BASE64-DECODED-STR> , <BASE64-DECODED-STR> should be non-empty.\n", argv[0], argv[1], argv[2]);
                return PPKG_ERROR_ARG_IS_NULL;
            }

            unsigned int  outputBufSizeInBytes = (inputBufSizeInBytes >> 2) * 3;
            unsigned char outputBuf[outputBufSizeInBytes];

            // EVP_DecodeBlock() returns the length of the data decoded or -1 on error.
            int n = EVP_DecodeBlock(outputBuf, inputBuf, inputBufSizeInBytes);

            if (n < 0) {
                return PPKG_ERROR_ARG_IS_INVALID;
            }

            if (fwrite(outputBuf, 1, n, stdout) != (size_t)n || ferror(stdout)) {
                return PPKG_ERROR;
            }

            if (isatty(STDOUT_FILENO)) {
                printf("\n");
            }

            return PPKG_OK;
        }
    }

    if (strcmp(argv[2], "sha256sum") == 0) {
        if (argv[3] == NULL || strcmp(argv[3], "-") == 0) {
            char outputBuf[65];
            outputBuf[64] = '\0';

            if (sha256sum_of_stream(outputBuf, stdin) != 0) {
                perror(NULL);
                return PPKG_ERROR;
            } else {
                printf("%s\n", outputBuf);
                return PPKG_OK;
            }
        } else if (strcmp(argv[3], "-h") == 0 || strcmp(argv[3], "--help") == 0) {
            fprintf(stderr, "Usage: %s %s %s [FILEPATH]\n", argv[0], argv[1], argv[2]);
            return PPKG_OK;
        } else {
            char outputBuf[65];
            outputBuf[64] = '\0';

            if (sha256sum_of_file(outputBuf, argv[3]) != 0) {
                perror(argv[3]);
                return PPKG_ERROR;
            } else {
                printf("%s\n", outputBuf);
                return PPKG_OK;
            }
        }
    }

    if (strcmp(argv[2], "zlib-deflate") == 0) {
        int level = 1;

        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "-L") == 0) {
                char * p = argv[i + 1];

                if (p == NULL) {
                    fprintf(stderr, "Usage: %s %s %s [-L N] (N>=1 && N <=9) , The smaller the N, the faster the speed and the lower the compression ratio.\n", argv[0], argv[1], argv[2]);
                    return PPKG_ERROR;
                }

                if (strlen(p) != 1) {
                    fprintf(stderr, "Usage: %s %s %s [-L N] (N>=1 && N <=9) , The smaller the N, the faster the speed and the lower the compression ratio.\n", argv[0], argv[1], argv[2]);
                    return PPKG_ERROR;
                }

                if (p[0] < '1' || p[0] > '9') {
                    fprintf(stderr, "Usage: %s %s %s [-L N] (N>=1 && N <=9) , The smaller the N, the faster the speed and the lower the compression ratio.\n", argv[0], argv[1], argv[2]);
                    return PPKG_ERROR;
                }

                level = atoi(p);

                i++;
            } else {
                fprintf(stderr, "unrecognized option: %s", argv[i]);
                fprintf(stderr, "Usage: %s %s %s [-L N] (N>=1 && N <=9) , The smaller the N, the faster the speed and the lower the compression ratio.\n", argv[0], argv[1], argv[2]);
                return PPKG_ERROR;
            }
        }

        return zlib_deflate_file_to_file(stdin, stdout, level);
    }

    if (strcmp(argv[2], "zlib-inflate") == 0) {
        return zlib_inflate_file_to_file(stdin, stdout);
    }

    if (strcmp(argv[2], "which") == 0) {
        if (argv[3] == NULL) {
            fprintf(stderr, "USAGE: %s %s %s <COMMAND-NAME> [-a]\n", argv[0], argv[1], argv[2]);
            return 1;
        }

        bool findAll = false;

        for (int i = 4; i < argc; i++) {
            if (strcmp(argv[i], "-a") == 0) {
                findAll = true;
            } else {
                fprintf(stderr, "unrecognized argument: %s\n", argv[i]);
                fprintf(stderr, "USAGE: %s %s %s <COMMAND-NAME> [-a]\n", argv[0], argv[1], argv[2]);
                return 1;
            }
        }

        char ** pathList = NULL;
        size_t  pathListSize = 0;

        int ret = exe_search(argv[3], &pathList, &pathListSize, findAll);

        if (ret == 0) {
            for (size_t i = 0; i < pathListSize; i++) {
                printf("%s\n", pathList[i]);

                free(pathList[i]);
                pathList[i] = NULL;
            }

            free(pathList);
            pathList = NULL;
        }

        return ret;
    }

    LOG_ERROR2("unrecognized command: ", argv[2]);
    return PPKG_ERROR_ARG_IS_UNKNOWN;
}
