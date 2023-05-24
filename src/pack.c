#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "core/log.h"
#include "core/tar.h"
#include "core/cp.h"
#include "core/sysinfo.h"

#include "ppkg.h"

int ppkg_pack(const char * packageName, ArchiveType type, bool verbose) {
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

    size_t  packingDirNameLength = strlen(packageName) + strlen(receipt->version) + strlen(osType) + strlen(osArch) + 4U;
    char    packingDirName[packingDirNameLength];
    snprintf(packingDirName, packingDirNameLength, "%s-%s-%s-%s", packageName, receipt->version, osType, osArch);

    ppkg_receipt_free(receipt);

    ///////////////////////////////////////////////////////////////////////////////////

    char * userHomeDir = getenv("HOME");

    if (userHomeDir == NULL) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t userHomeDirLength = strlen(userHomeDir);

    if (userHomeDirLength == 0) {
        return PPKG_ERROR_ENV_HOME_NOT_SET;
    }

    size_t  ppkgHomeDirLength = userHomeDirLength + 7U;
    char    ppkgHomeDir[ppkgHomeDirLength];
    snprintf(ppkgHomeDir, ppkgHomeDirLength, "%s/.ppkg", userHomeDir);

    size_t  packageInstalledDirLength = ppkgHomeDirLength + strlen(packageName) + 12U;
    char    packageInstalledDir[packageInstalledDirLength];
    snprintf(packageInstalledDir, packageInstalledDirLength, "%s/installed/%s", ppkgHomeDir, packageName);

    size_t  receiptFilePathLength = packageInstalledDirLength + 20U;
    char    receiptFilePath[receiptFilePathLength];
    snprintf(receiptFilePath, receiptFilePathLength, "%s/.ppkg/receipt.yml", packageInstalledDir);

    struct stat st;

    if (stat(receiptFilePath, &st) != 0 || !S_ISREG(st.st_mode)) {
        return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    /////////////////////////////////////////////////////////////////////////////////

    size_t  ppkgPackingDirLength = ppkgHomeDirLength + 9U;
    char    ppkgPackingDir[ppkgPackingDirLength];
    snprintf(ppkgPackingDir, ppkgPackingDirLength, "%s/packing", ppkgHomeDir);

    if (stat(ppkgPackingDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", ppkgPackingDir);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(ppkgPackingDir, S_IRWXU) != 0) {
            perror(ppkgPackingDir);
            return PPKG_ERROR;
        }
    }

    if (chdir(ppkgPackingDir) != 0) {
        perror(ppkgPackingDir);
        return PPKG_ERROR;
    }

    if (stat(packingDirName, &st) == 0) {
        if (S_ISLNK(st.st_mode)) {
            if (unlink(packingDirName) != 0) {
                perror(packingDirName);
                return PPKG_ERROR;
            }
        }
    }

    if (symlink(packageInstalledDir, packingDirName) != 0) {
        perror(packingDirName);
        return PPKG_ERROR;
    }

    char * archiveFileType;

    switch (type) {
        case ArchiveType_tar_gz:  archiveFileType = (char*)".tar.gz";  break;
        case ArchiveType_tar_xz:  archiveFileType = (char*)".tar.xz";  break;
        case ArchiveType_tar_lz:  archiveFileType = (char*)".tar.lz";  break;
        case ArchiveType_tar_bz2: archiveFileType = (char*)".tar.bz2"; break;
        case ArchiveType_zip:     archiveFileType = (char*)".zip";     break;
    }

    size_t  stageArchiveFilePathLength = ppkgPackingDirLength + packingDirNameLength + 10U;
    char    stageArchiveFilePath[stageArchiveFilePathLength];
    snprintf(stageArchiveFilePath, stageArchiveFilePathLength, "%s/%s%s", ppkgPackingDir, packingDirName, archiveFileType);

    ret = tar_create(packingDirName, stageArchiveFilePath, type, verbose);

    if (ret != 0) {
        return ret;
    }

    ///////////////////////////////////////////////////////////////////////////////////

    struct stat sb;

    if (stat(stageArchiveFilePath, &sb) != 0) {
        perror(stageArchiveFilePath);
        return PPKG_ERROR;
    }

    off_t nBytes = sb.st_size;

    if (nBytes < 1024) {
        printf("%s %lu Byte\n", stageArchiveFilePath, nBytes);
    } else if (nBytes < 1024 * 1024) {
        printf("%s %lu KB\n", stageArchiveFilePath, nBytes / 1024);
    } else if (nBytes < 1024 * 1024 * 1024) {
        printf("%s %lu MB\n", stageArchiveFilePath, nBytes / 1024 / 1024);
    } else {
        LOG_ERROR2("file is too large: ", stageArchiveFilePath);
        return PPKG_ERROR;
    }

    ///////////////////////////////////////////////////////////////////////////////////

    size_t  ppkgPackedDirLength = strlen(userHomeDir) + 15U;
    char    ppkgPackedDir[ppkgPackedDirLength];
    snprintf(ppkgPackedDir, ppkgPackedDirLength, "%s/.ppkg/packed", userHomeDir);

    if (stat(ppkgPackedDir, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "'%s\n' was expected to be a directory, but it was not.\n", ppkgPackedDir);
            return PPKG_ERROR;
        }
    } else {
        if (mkdir(ppkgPackedDir, S_IRWXU) != 0) {
            perror(ppkgPackedDir);
            return PPKG_ERROR;
        }
    }

    size_t  finalArchiveFilePathLength = ppkgPackedDirLength + packingDirNameLength + 10U;
    char    finalArchiveFilePath[finalArchiveFilePathLength];
    snprintf(finalArchiveFilePath, finalArchiveFilePathLength, "%s/%s%s", ppkgPackedDir, packingDirName, archiveFileType);

    return copy_file(stageArchiveFilePath, finalArchiveFilePath);
}
