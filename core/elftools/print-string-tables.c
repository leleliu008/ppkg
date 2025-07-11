#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <elf.h>


int handle_elf32(const int fd, const char * const fp) {
    Elf32_Ehdr ehdr;

    int ret = read(fd, &ehdr, sizeof(Elf32_Ehdr));

    if (ret == -1) {
        perror(fp);
        return 11;
    }

    if (ret != sizeof(Elf32_Ehdr)) {
        perror(fp);
        return 12;
    }

    ///////////////////////////////////////////////////////////

    Elf32_Shdr shdr;

    // .shstrtab section header offset in elf file, it usually is the last section header
    off_t offset = ehdr.e_shoff + ehdr.e_shstrndx * ehdr.e_shentsize;

    ret = pread(fd, &shdr, sizeof(Elf32_Shdr), offset);

    if (ret == -1) {
        perror(fp);
        return 13;
    }

    if (ret != sizeof(Elf32_Shdr)) {
        perror(fp);
        fprintf(stderr, "not fully read.\n");
        return 14;
    }

    char shstrtab[shdr.sh_size];

    ret = pread(fd, shstrtab, shdr.sh_size, shdr.sh_offset);

    if (ret == -1) {
        perror(fp);
        return 15;
    }

    if ((size_t)ret != shdr.sh_size) {
        perror(fp);
        fprintf(stderr, "not fully read.\n");
        return 16;
    }

    ///////////////////////////////////////////////////////////

    for (unsigned int i = 1; i < ehdr.e_shnum; i++) {
        ret = pread(fd, &shdr, sizeof(Elf32_Shdr), ehdr.e_shoff + i * ehdr.e_shentsize);

        if (ret == -1) {
            perror(fp);
            return 17;
        }

        if ((size_t)ret != sizeof(Elf32_Shdr)) {
            perror(fp);
            fprintf(stderr, "not fully read.\n");
            return 18;
        }

        if (shdr.sh_type == SHT_STRTAB) {
            printf("%s:\n", shstrtab + shdr.sh_name);

            char buf[shdr.sh_size];

            ret = pread(fd, buf, shdr.sh_size, shdr.sh_offset);

            if (ret == -1) {
                perror(fp);
                return 15;
            }

            if ((size_t)ret != shdr.sh_size) {
                perror(fp);
                fprintf(stderr, "not fully read.\n");
                return 16;
            }

            char * p = buf + 1;

            // https://www.sco.com/developers/gabi/latest/ch4.strtab.html
            for (unsigned int j = 1; j < shdr.sh_size; j++) {
                if (buf[j] == '\0') {
                    printf("    %s\n", p);
                    p = buf + j + 1;
                }
            }
        }
    }

    return 0;
}

int handle_elf64(const int fd, const char * const fp) {
    Elf64_Ehdr ehdr;

    int ret = read(fd, &ehdr, sizeof(Elf64_Ehdr));

    if (ret == -1) {
        perror(fp);
        return 11;
    }

    if (ret != sizeof(Elf64_Ehdr)) {
        perror(fp);
        return 12;
    }

    ///////////////////////////////////////////////////////////

    Elf64_Shdr shdr;

    // .shstrtab section header offset in elf file, it usually is the last section header
    off_t offset = ehdr.e_shoff + ehdr.e_shstrndx * ehdr.e_shentsize;

    ret = pread(fd, &shdr, sizeof(Elf64_Shdr), offset);

    if (ret == -1) {
        perror(fp);
        return 13;
    }

    if (ret != sizeof(Elf64_Shdr)) {
        perror(fp);
        fprintf(stderr, "not fully read.\n");
        return 14;
    }

    char shstrtab[shdr.sh_size];

    ret = pread(fd, shstrtab, shdr.sh_size, shdr.sh_offset);

    if (ret == -1) {
        perror(fp);
        return 15;
    }

    if ((size_t)ret != shdr.sh_size) {
        perror(fp);
        fprintf(stderr, "not fully read.\n");
        return 16;
    }

    ///////////////////////////////////////////////////////////

    for (unsigned int i = 1; i < ehdr.e_shnum; i++) {
        ret = pread(fd, &shdr, sizeof(Elf64_Shdr), ehdr.e_shoff + i * ehdr.e_shentsize);

        if (ret == -1) {
            perror(fp);
            return 17;
        }

        if ((size_t)ret != sizeof(Elf64_Shdr)) {
            perror(fp);
            fprintf(stderr, "not fully read.\n");
            return 18;
        }

        if (shdr.sh_type == SHT_STRTAB) {
            printf("%s:\n", shstrtab + shdr.sh_name);

            char buf[shdr.sh_size];

            ret = pread(fd, buf, shdr.sh_size, shdr.sh_offset);

            if (ret == -1) {
                perror(fp);
                return 15;
            }

            if ((size_t)ret != shdr.sh_size) {
                perror(fp);
                fprintf(stderr, "not fully read.\n");
                return 16;
            }

            char * p = buf + 1;

            // https://www.sco.com/developers/gabi/latest/ch4.strtab.html
            for (unsigned int j = 1; j < shdr.sh_size; j++) {
                if (buf[j] == '\0') {
                    printf("    %s\n", p);
                    p = buf + j + 1;
                }
            }
        }
    }

    return 0;
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

    if (st.st_size < 52) {
        fprintf(stderr, "NOT an ELF file: %s\n", argv[1]);
        close(fd);
        return 100;
    }

    ///////////////////////////////////////////////////////////

    unsigned char a[5];

    ssize_t readBytes = read(fd, a, 5);

    if (readBytes == -1) {
        perror(argv[0]);
        close(fd);
        return 5;
    }

    if (readBytes != 5) {
        perror(argv[0]);
        close(fd);
        fprintf(stderr, "not fully read.\n");
        return 6;
    }

    ///////////////////////////////////////////////////////////

    // https://www.sco.com/developers/gabi/latest/ch4.eheader.html
    if ((a[0] != 0x7F) || (a[1] != 0x45) || (a[2] != 0x4C) || (a[3] != 0x46)) {
        fprintf(stderr, "NOT an ELF file: %s\n", argv[1]);
        close(fd);
        return 100;
    }

    ///////////////////////////////////////////////////////////

    off_t offset = lseek(fd, 0, SEEK_SET);

    if (offset == -1) {
        perror(argv[0]);
        close(fd);
        return 7;
    }

    ///////////////////////////////////////////////////////////

    int ret;

    switch (a[4]) {
        case ELFCLASS32: ret = handle_elf32(fd, argv[1]); break;
        case ELFCLASS64: ret = handle_elf64(fd, argv[1]); break;
        default: 
            fprintf(stderr, "Invalid ELF file: %s\n", argv[1]);
            ret = 101;
    }

    close(fd);

    return ret;
}
