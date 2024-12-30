#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <elf.h>


int handle_elf32(const int fd, const char * const fp) {
    Elf32_Ehdr ehdr;

    int ret = read(fd, &ehdr, sizeof(Elf32_Ehdr));

    if (ret == -1) {
        perror(fp);
        close(fd);
        return 11;
    }

    if (ret != sizeof(Elf32_Ehdr)) {
        perror(fp);
        close(fd);
        return 12;
    }

    ///////////////////////////////////////////////////////////

    Elf32_Shdr shdr;

    // .shstrtab section header offset in elf file, it usually is the last section header
    off_t offset = ehdr.e_shoff + ehdr.e_shstrndx * ehdr.e_shentsize;

    ret = pread(fd, &shdr, sizeof(Elf32_Shdr), offset);

    if (ret == -1) {
        perror(fp);
        close(fd);
        return 13;
    }

    if (ret != sizeof(Elf32_Shdr)) {
        perror(fp);
        close(fd);
        fprintf(stderr, "not fully read.\n");
        return 14;
    }

    char strings[shdr.sh_size];

    ret = pread(fd, strings, shdr.sh_size, shdr.sh_offset);

    if (ret == -1) {
        perror(fp);
        close(fd);
        return 15;
    }

    if ((size_t)ret != shdr.sh_size) {
        perror(fp);
        close(fd);
        fprintf(stderr, "not fully read.\n");
        return 16;
    }

    char * p = &strings[1];

    // https://docs.oracle.com/cd/E23824_01/html/819-0690/chapter6-73709.html
    for (unsigned int i = 1; i < shdr.sh_size; i++) {
        if (strings[i] == '\0') {
            if (strcmp(p, ".dynamic") == 0) {
                return 0;
            }
            p = &strings[i + 1];
        }
    }

    fprintf(stderr, "There is no dynamic section in this file.\n");
    return 100;
}

int handle_elf64(const int fd, const char * const fp) {
    Elf64_Ehdr ehdr;

    int ret = read(fd, &ehdr, sizeof(Elf64_Ehdr));

    if (ret == -1) {
        perror(fp);
        close(fd);
        return 11;
    }

    if (ret != sizeof(Elf64_Ehdr)) {
        perror(fp);
        close(fd);
        return 12;
    }

    ///////////////////////////////////////////////////////////

    Elf64_Shdr shdr;

    // .shstrtab section header offset in elf file, it usually is the last section header
    off_t offset = ehdr.e_shoff + ehdr.e_shstrndx * ehdr.e_shentsize;

    ret = pread(fd, &shdr, sizeof(Elf64_Shdr), offset);

    if (ret == -1) {
        perror(fp);
        close(fd);
        return 13;
    }

    if (ret != sizeof(Elf64_Shdr)) {
        perror(fp);
        close(fd);
        fprintf(stderr, "not fully read.\n");
        return 14;
    }

    char strings[shdr.sh_size];

    ret = pread(fd, strings, shdr.sh_size, shdr.sh_offset);

    if (ret == -1) {
        perror(fp);
        close(fd);
        return 15;
    }

    if ((size_t)ret != shdr.sh_size) {
        perror(fp);
        close(fd);
        fprintf(stderr, "not fully read.\n");
        return 16;
    }

    char * p = &strings[1];

    // https://docs.oracle.com/cd/E23824_01/html/819-0690/chapter6-73709.html
    for (unsigned int i = 1; i < shdr.sh_size; i++) {
        if (strings[i] == '\0') {
            if (strcmp(p, ".dynamic") == 0) {
                return 0;
            }
            p = &strings[i + 1];
        }
    }

    fprintf(stderr, "There is no dynamic section in this file.\n");
    return 100;
}

int main(int argc, const char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s <ELF-FILEPATH>\n", argv[0]);
		return 1;
	}

    if (argv[1][0] == '\0') {
		printf("Usage: %s <ELF-FILEPATH>, <ELF-FILEPATH> is unspecified.\n", argv[0]);
		return 2;
    }

    int fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        perror(argv[1]);
        return 3;
    }

    struct stat st;

    if (fstat(fd, &st) == -1) {
        perror(argv[1]);
        close(fd);
        return 4;
    }

    if (st.st_size < 5) {
        fprintf(stderr, "NOT an ELF file: %s\n", argv[1]);
        return 5;
    }

    ///////////////////////////////////////////////////////////

    unsigned char a[5];

    ssize_t readCount = read(fd, a, 5);

    if (readCount == -1) {
        perror(argv[0]);
        close(fd);
        return 6;
    }

    if (readCount != 5) {
        perror(argv[0]);
        close(fd);
        fprintf(stderr, "not fully read.\n");
        return 7;
    }

    ///////////////////////////////////////////////////////////

    // https://www.sco.com/developers/gabi/latest/ch4.eheader.html
    if ((a[0] != 0x7F) || (a[1] != 0x45) || (a[2] != 0x4C) || (a[3] != 0x46)) {
        fprintf(stderr, "NOT an ELF file: %s\n", argv[1]);
        return 8;
    }

    ///////////////////////////////////////////////////////////

    off_t offset = lseek(fd, 0, SEEK_SET);

    if (offset == -1) {
        perror(argv[0]);
        close(fd);
        return 9;
    }

    ///////////////////////////////////////////////////////////

    switch (a[4]) {
        case 1: return handle_elf32(fd, argv[1]);
        case 2: return handle_elf64(fd, argv[1]);
        default: 
            fprintf(stderr, "Invalid ELF file: %s\n", argv[1]);
            return 10;
    }
}
