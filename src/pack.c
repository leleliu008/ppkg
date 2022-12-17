#include <string.h>
#include <unistd.h>

#include "ppkg.h"
#include "core/fs.h"
#include "core/log.h"
#include "core/untar.h"
#include "core/sysinfo.h"

int ppkg_pack(const char * packageName, ArchiveType type, bool verbose) {
    PPKGReceipt * receipt = NULL;

    int resultCode = ppkg_receipt_parse(packageName, &receipt);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    ///////////////////////////////////////////////////////////////////////////////////

    char * osType = NULL;
    char * osArch = NULL;

    if (sysinfo_type(&osType) != 0) {
        ppkg_receipt_free(receipt);
        return PPKG_ERROR;
    }

    if (sysinfo_arch(&osArch) != 0) {
        ppkg_receipt_free(receipt);
        return PPKG_ERROR;
    }

    size_t  packingDirNameLength = strlen(packageName) + strlen(receipt->version) + strlen(osType) + strlen(osArch) + 4;
    char    packingDirName[packingDirNameLength];
    memset (packingDirName, 0, packingDirNameLength);
    sprintf(packingDirName, "%s-%s-%s-%s", packageName, receipt->version, osType, osArch);

    ppkg_receipt_free(receipt);
    free(osType);
    free(osArch);

    ///////////////////////////////////////////////////////////////////////////////////

    char * userHomeDir = getenv("HOME");

    if ((userHomeDir == NULL) || (strcmp(userHomeDir, "") == 0)) {
        return PPKG_ENV_HOME_NOT_SET;
    }

    size_t  ppkgPackingDirLength = strlen(userHomeDir) + 15;
    char    ppkgPackingDir[ppkgPackingDirLength];
    memset (ppkgPackingDir, 0, ppkgPackingDirLength);
    sprintf(ppkgPackingDir, "%s/.ppkg/packing", userHomeDir);

    if (!exists_and_is_a_directory(ppkgPackingDir)) {
        if (mkdir(ppkgPackingDir, S_IRWXU) != 0) {
            perror(ppkgPackingDir);
            return PPKG_ERROR;
        }
    }

    if (chdir(ppkgPackingDir) != 0) {
        perror(ppkgPackingDir);
        return PPKG_ERROR;
    }

    size_t  packageInstalledDirLength = strlen(packageName) + 21;
    char    packageInstalledDir[packageInstalledDirLength];
    memset (packageInstalledDir, 0, packageInstalledDirLength);
    sprintf(packageInstalledDir, "/opt/ppkg/installed/%s", packageName);

    if (exists_and_is_a_directory(packingDirName)) {
        if (unlink(packingDirName) != 0) {
            perror(packingDirName);
            return PPKG_ERROR;
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

    size_t  stageArchiveFilePathLength = ppkgPackingDirLength + packingDirNameLength + 10;
    char    stageArchiveFilePath[stageArchiveFilePathLength];
    memset (stageArchiveFilePath, 0, stageArchiveFilePathLength);
    sprintf(stageArchiveFilePath, "%s/%s%s", ppkgPackingDir, packingDirName, archiveFileType);

    resultCode = untar_create(packingDirName, stageArchiveFilePath, 0, type, verbose);

    if (resultCode != PPKG_OK) {
        return resultCode;
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

    size_t  ppkgPackedDirLength = strlen(userHomeDir) + 15;
    char    ppkgPackedDir[ppkgPackedDirLength];
    memset (ppkgPackedDir, 0, ppkgPackedDirLength);
    sprintf(ppkgPackedDir, "%s/.ppkg/packed", userHomeDir);

    if (!exists_and_is_a_directory(ppkgPackedDir)) {
        if (mkdir(ppkgPackedDir, S_IRWXU) != 0) {
            perror(ppkgPackedDir);
            return PPKG_ERROR;
        }
    }

    size_t  finalArchiveFilePathLength = ppkgPackedDirLength + packingDirNameLength + 10;
    char    finalArchiveFilePath[finalArchiveFilePathLength];
    memset (finalArchiveFilePath, 0, finalArchiveFilePathLength);
    sprintf(finalArchiveFilePath, "%s/%s%s", ppkgPackedDir, packingDirName, archiveFileType);

    FILE * finalArchiveFile = fopen(finalArchiveFilePath, "r");

    if (finalArchiveFile == NULL) {
        perror(finalArchiveFilePath);
        return PPKG_ERROR;
    }

    FILE * stageArchiveFile = fopen(stageArchiveFilePath, "r");

    if (stageArchiveFile == NULL) {
        fclose(finalArchiveFile);
        perror(stageArchiveFilePath);
        return PPKG_ERROR;
    }

    unsigned char buff[1024];
    int  size = 0;
    while((size = fread(buff, 1, 1024, stageArchiveFile)) != 0) {
        fwrite(buff, 1, size, finalArchiveFile);
    }

    fclose(stageArchiveFile);
    fclose(finalArchiveFile);

    return resultCode;
}
