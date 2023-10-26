#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "core/log.h"
#include "core/tar.h"
#include "core/sysinfo.h"

#include "ppkg.h"

int ppkg_pack(const char * packageName, ArchiveType outputType, const char * outputPath, bool verbose) {
    PPKGReceipt * receipt = NULL;

    int ret = ppkg_receipt_parse(packageName, &receipt);

    if (ret != PPKG_OK) {
        return ret;
    }

    ///////////////////////////////////////////////////////////////////////////////////

    char osType[31] = {0};

    if (sysinfo_type(osType, 30) != 0) {
        ppkg_receipt_free(receipt);
        return PPKG_ERROR;
    }

    char osArch[31] = {0};

    if (sysinfo_arch(osArch, 30) != 0) {
        ppkg_receipt_free(receipt);
        return PPKG_ERROR;
    }

    size_t   packingDIRNameLength = strlen(packageName) + strlen(receipt->version) + strlen(osType) + strlen(osArch) + 4U;
    char     packingDIRName[packingDIRNameLength];
    snprintf(packingDIRName, packingDIRNameLength, "%s-%s-%s-%s", packageName, receipt->version, osType, osArch);

    ppkg_receipt_free(receipt);

    /////////////////////////////////////////////////////////////////////////////////

    const char * outputFileExt;

    switch (outputType) {
        case ArchiveType_tar_gz:  outputFileExt = (char*)".tar.gz";  break;
        case ArchiveType_tar_xz:  outputFileExt = (char*)".tar.xz";  break;
        case ArchiveType_tar_lz:  outputFileExt = (char*)".tar.lz";  break;
        case ArchiveType_tar_bz2: outputFileExt = (char*)".tar.bz2"; break;
        case ArchiveType_zip:     outputFileExt = (char*)".zip";     break;
    }

    /////////////////////////////////////////////////////////////////////////////////

    char cwd[PATH_MAX];

    char * p = getcwd(cwd, PATH_MAX);

    if (p == NULL) {
        perror(NULL);
        return PPKG_ERROR;
    }

    /////////////////////////////////////////////////////////////////////////////////

    char outputFilePath[PATH_MAX];

    if (outputPath == NULL) {
        snprintf(outputFilePath, PATH_MAX, "%s/%s%s", cwd, packingDIRName, outputFileExt);
    } else {
        struct stat st;

        if (stat(outputPath, &st) == 0 && S_ISDIR(st.st_mode)) {
            if (outputPath[0] == '/') {
                snprintf(outputFilePath, PATH_MAX, "%s/%s%s", outputPath, packingDIRName, outputFileExt);
            } else {
                snprintf(outputFilePath, PATH_MAX, "%s/%s/%s%s", cwd, outputPath, packingDIRName, outputFileExt);
            }
        } else {
            size_t outputPathLength = strlen(outputPath);

            if (outputPath[outputPathLength - 1U] == '/') {
                if (outputPath[0] == '/') {
                    snprintf(outputFilePath, PATH_MAX, "%s%s%s", outputPath, packingDIRName, outputFileExt);
                } else {
                    snprintf(outputFilePath, PATH_MAX, "%s/%s%s%s", cwd, outputPath, packingDIRName, outputFileExt);
                }
            } else {
                if (outputPath[0] == '/') {
                    strncpy(outputFilePath, outputPath, outputPathLength);
                    outputFilePath[outputPathLength] = '\0';
                } else {
                    snprintf(outputFilePath, PATH_MAX, "%s/%s", cwd, outputPath);
                }
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////

    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    ret = ppkg_home_dir(ppkgHomeDIR, PATH_MAX, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    char   sessionDIR[PATH_MAX] = {0};
    size_t sessionDIRLength;

    ret = ppkg_session_dir(sessionDIR, PATH_MAX, &sessionDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    /////////////////////////////////////////////////////////////////////////////////

    if (chdir(sessionDIR) != 0) {
        perror(sessionDIR);
        return PPKG_ERROR;
    }

    /////////////////////////////////////////////////////////////////////////////////

    size_t   packageInstalledDIRLength = ppkgHomeDIRLength + strlen(packageName) + 12U;
    char     packageInstalledDIR[packageInstalledDIRLength];
    snprintf(packageInstalledDIR, packageInstalledDIRLength, "%s/installed/%s", ppkgHomeDIR, packageName);

    if (symlink(packageInstalledDIR, packingDIRName) != 0) {
        perror(packageInstalledDIR);
        return PPKG_ERROR;
    }

    /////////////////////////////////////////////////////////////////////////////////

    size_t   tmpFilePathLength = sessionDIRLength + packingDIRNameLength + 10U;
    char     tmpFilePath[tmpFilePathLength];
    snprintf(tmpFilePath, tmpFilePathLength, "%s/%s%s", sessionDIR, packingDIRName, outputFileExt);

    ret = tar_create(packingDIRName, tmpFilePath, outputType, verbose);

    if (ret != 0) {
        return ret;
    }

    ret = ppkg_rename_or_copy_file(tmpFilePath, outputFilePath);

    if (ret != PPKG_OK) {
        return ret;
    }

    /////////////////////////////////////////////////////////////////////////////////

    struct stat st;

    if (stat(outputFilePath, &st) != 0) {
        perror(outputFilePath);
        return PPKG_ERROR;
    }

    off_t nBytes = st.st_size;

    if (nBytes < 1024) {
        printf("%s %jd Byte\n", outputFilePath, nBytes);
    } else if (nBytes < 1024 * 1024) {
        printf("%s %jd KB\n", outputFilePath, nBytes / 1024);
    } else if (nBytes < 1024 * 1024 * 1024) {
        printf("%s %jd MB\n", outputFilePath, nBytes / 1024 / 1024);
    } else {
        LOG_ERROR2("file is too large: ", tmpFilePath);
        return PPKG_ERROR;
    }

    return ppkg_rm_r(sessionDIR, verbose);
}
