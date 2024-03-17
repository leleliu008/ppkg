#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "core/log.h"
#include "core/tar.h"

#include "ppkg.h"

int ppkg_pack(const char * packageName, const PPKGTargetPlatform * targetPlatform, ArchiveType outputType, const char * outputPath, const bool verbose) {
    if (targetPlatform == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    PPKGReceipt * receipt = NULL;

    int ret = ppkg_receipt_parse(packageName, targetPlatform, &receipt);

    if (ret != PPKG_OK) {
        return ret;
    }

    ///////////////////////////////////////////////////////////////////////////////////

    size_t packingDIRNameCapacity = strlen(packageName) + strlen(receipt->version) + strlen(receipt->builtFor) + 3U;
    char   packingDIRName[packingDIRNameCapacity];

    ret = snprintf(packingDIRName, packingDIRNameCapacity, "%s-%s-%s", packageName, receipt->version, receipt->builtFor);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    ppkg_receipt_free(receipt);

    /////////////////////////////////////////////////////////////////////////////////

    const char * outputFileExt;

    switch (outputType) {
        case ArchiveType_tar_gz:  outputFileExt = ".tar.gz";  break;
        case ArchiveType_tar_xz:  outputFileExt = ".tar.xz";  break;
        case ArchiveType_tar_lz:  outputFileExt = ".tar.lz";  break;
        case ArchiveType_tar_bz2: outputFileExt = ".tar.bz2"; break;
        case ArchiveType_zip:     outputFileExt = ".zip";     break;
    }

    /////////////////////////////////////////////////////////////////////////////////

    char buf[PATH_MAX];

    char * cwd = getcwd(buf, PATH_MAX);

    if (cwd == NULL) {
        perror(NULL);
        return PPKG_ERROR;
    }

    /////////////////////////////////////////////////////////////////////////////////

    char outputFilePath[PATH_MAX];

    if (outputPath == NULL) {
        ret = snprintf(outputFilePath, PATH_MAX, "%s/%s%s", cwd, packingDIRName, outputFileExt);
    } else {
        size_t outputPathLength = strlen(outputPath);

        if (outputPath[outputPathLength - 1U] == '/') {
            if (outputPath[0] == '/') {
                ret = snprintf(outputFilePath, PATH_MAX, "%s%s%s", outputPath, packingDIRName, outputFileExt);
            } else {
                ret = snprintf(outputFilePath, PATH_MAX, "%s/%s%s%s", cwd, outputPath, packingDIRName, outputFileExt);
            }
        } else {
            if (outputPath[0] == '/') {
                strncpy(outputFilePath, outputPath, outputPathLength);
                outputFilePath[outputPathLength] = '\0';
            } else {
                ret = snprintf(outputFilePath, PATH_MAX, "%s/%s", cwd, outputPath);
            }
        }
    }

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
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

    size_t packageInstalledDIRCapacity = ppkgHomeDIRLength + targetPlatform->nameLen + targetPlatform->versLen + targetPlatform->archLen + strlen(packageName) + 15U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/installed/%s-%s-%s/%s", ppkgHomeDIR, targetPlatform->name, targetPlatform->vers, targetPlatform->arch, packageName);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (symlink(packageInstalledDIR, packingDIRName) != 0) {
        perror(packageInstalledDIR);
        return PPKG_ERROR;
    }

    /////////////////////////////////////////////////////////////////////////////////

    size_t tmpFilePathCapacity = sessionDIRLength + packingDIRNameCapacity + 10U;
    char   tmpFilePath[tmpFilePathCapacity];

    ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/%s%s", sessionDIR, packingDIRName, outputFileExt);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

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
        if (sizeof(off_t) == sizeof(long long)) {
            printf("%s %lld Byte\n", outputFilePath, (long long)nBytes);
        } else {
            printf("%s %ld Byte\n", outputFilePath, (long int)nBytes);
        }
    } else if (nBytes < 1024 * 1024) {
        printf("%s %.2f KB\n", outputFilePath, nBytes / 1024.0);
    } else if (nBytes < 1024 * 1024 * 1024) {
        printf("%s %.2f MB\n", outputFilePath, nBytes / 1024.0 / 1024.0);
    } else {
        LOG_ERROR2("file is too large: ", tmpFilePath);
        return PPKG_ERROR;
    }

    return ppkg_rm_rf(sessionDIR, false, verbose);
}
