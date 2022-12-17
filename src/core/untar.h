#ifndef UNTAR_H
#define UNTAR_H

    #include <stdlib.h>
    #include <stdbool.h>

    #include <archive.h>
    #include <archive_entry.h>

    #ifdef __cplasplas
        extern "C"
    #endif

        typedef enum {
            ArchiveType_tar_gz,
            ArchiveType_tar_xz,
            ArchiveType_tar_lz,
            ArchiveType_tar_bz2,
            ArchiveType_zip,
        } ArchiveType;

        int untar_list(const char * inputFilePath, int flags);

        int untar_create(const char * inputDir, const char * outputFilePath, int flags, ArchiveType type, bool verbose);

        int untar_extract(const char * outputDir, const char * inputFilePath, int flags, bool verbose, size_t stripComponentsNumber);

    #ifdef __cplasplas
        }
    #endif
#endif
