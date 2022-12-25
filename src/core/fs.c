#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include "fs.h"

bool exists_and_is_a_directory(const char* dirpath) {
    struct stat sb;
    return (stat(dirpath, &sb) == 0) && S_ISDIR(sb.st_mode);
}

bool exists_and_is_a_symbolic_link(const char* filepath) {
    struct stat sb;
    return (stat(filepath, &sb) == 0) && S_ISLNK(sb.st_mode);
}

bool exists_and_is_a_regular_file(const char* filepath) {
    struct stat sb;
    return (stat(filepath, &sb) == 0) && S_ISREG(sb.st_mode);
}

bool exists_and_is_readable(const char* filepath) {
    return access(filepath, R_OK) == 0;
}

bool exists_and_is_writable(const char* filepath) {
    return access(filepath, W_OK) == 0;
}

bool exists_and_is_executable(const char* filepath) {
    return access(filepath, X_OK) == 0;
}

int cp(const char * srcFilePath, const char * dstFilePath) {
    FILE * dstFile = fopen(dstFilePath, "w");

    if (dstFile == NULL) {
        perror(dstFilePath);
        return -1;
    }

    FILE * srcFile = fopen(srcFilePath, "r");

    if (srcFile == NULL) {
        fclose(dstFile);
        perror(srcFilePath);
        return -1;
    }

    char   buff[1024];
    size_t size = 0;
    while((size = fread(buff, 1, 1024, srcFile)) != 0) {
        fwrite(buff, 1, size, dstFile);
    }

    fclose(srcFile);
    fclose(dstFile);

    return 0;
}
