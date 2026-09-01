#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/mman.h>

#include "elf.h"


static int handle_elf32(const unsigned char * elf) {
    Elf32_Ehdr * ehdr = (Elf32_Ehdr*)elf;

    for (Elf32_Half i = 0; i < ehdr->e_phnum; i++) {
        Elf32_Phdr * phdr = (Elf32_Phdr*)(elf + ehdr->e_phoff + i * ehdr->e_phentsize);

        if (phdr->p_type == PT_INTERP) {
            return 0;
        }
    }

    return 1;
}

static int handle_elf32_swap(const unsigned char * elf) {
    Elf32_Ehdr * ehdr = (Elf32_Ehdr*)elf;

    uint16_t phnum = __builtin_bswap16(ehdr->e_phnum);
    uint32_t phoff = __builtin_bswap32(ehdr->e_phoff);
    uint16_t phentsize = __builtin_bswap16(ehdr->e_phentsize);

    for (uint16_t i = 0; i < phnum; i++) {
        Elf32_Phdr * phdr = (Elf32_Phdr*)(elf + phoff + i * phentsize);

        if (__builtin_bswap32(phdr->p_type) == PT_INTERP) {
            return 0;
        }
    }

    return 1;
}

static int handle_elf64(const unsigned char * elf) {
    Elf64_Ehdr * ehdr = (Elf64_Ehdr*)elf;

    for (Elf64_Half i = 0; i < ehdr->e_phnum; i++) {
        Elf64_Phdr * phdr = (Elf64_Phdr*)(elf + ehdr->e_phoff + i * ehdr->e_phentsize);

        if (phdr->p_type == PT_INTERP) {
            return 0;
        }
    }

    return 1;
}

static int handle_elf64_swap(const unsigned char * elf) {
    Elf64_Ehdr * ehdr = (Elf64_Ehdr*)elf;

    uint16_t phnum = __builtin_bswap16(ehdr->e_phnum);
    uint64_t phoff = __builtin_bswap64(ehdr->e_phoff);
    uint16_t phentsize = __builtin_bswap16(ehdr->e_phentsize);

    for (uint16_t i = 0; i < phnum; i++) {
        Elf64_Phdr * phdr = (Elf64_Phdr*)(elf + phoff + i * phentsize);

        if (__builtin_bswap32(phdr->p_type) == PT_INTERP) {
            return 0;
        }
    }

    return 1;
}

int main(int argc, char* argv[]) {
    const char * fp = argv[1];

    if (fp == NULL) {
        fprintf(stderr, "Usage : %s %s <FILEPATH>, <FILEPATH> is unspecified.\n", argv[0], argv[1]);
        return 1;
    }

    if (fp[0] == '\0') {
        fprintf(stderr, "Usage : %s %s <FILEPATH>, <FILEPATH> should be a non-empty string.\n", argv[0], argv[1]);
        return 1;
    }

    int fd = open(fp, O_RDONLY);

    if (fd == -1) {
        perror(fp);
        return 2;
    }

    struct stat st;

    if (fstat(fd, &st) == -1) {
        perror(fp);
        close(fd);
        return 3;
    }

    if (st.st_size < 52) {
        fprintf(stderr, "NOT an ELF file: %s\n", fp);
        close(fd);
        return 0;
    }

    ///////////////////////////////////////////////////////////

    unsigned char a[6];

    ssize_t readBytes = read(fd, a, 6);

    if (readBytes == -1) {
        perror(fp);
        close(fd);
        return 4;
    }

    if (readBytes != 6) {
        perror(fp);
        close(fd);
        fprintf(stderr, "not fully read.\n");
        return 5;
    }

    ///////////////////////////////////////////////////////////

    // https://www.sco.com/developers/gabi/latest/ch4.eheader.html
    if ((a[0] != 0x7F) || (a[1] != 0x45) || (a[2] != 0x4C) || (a[3] != 0x46)) {
        fprintf(stderr, "NOT an ELF file: %s\n", fp);
        close(fd);
        return 0;
    }

    ///////////////////////////////////////////////////////////

    int swap = 0;

    switch (a[5]) {
        case ELFDATA2LSB:
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            swap = 1;
#endif
            break;
        case ELFDATA2MSB:
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            swap = 1;
#endif
            break;
        default:
            fprintf(stderr, "Invalid ELF file: %s\n", fp);
            return 10;
    }

    ///////////////////////////////////////////////////////////

    void * p = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (p == MAP_FAILED) {
        perror(fp);
        close(fd);
        return 5;
    }

    const unsigned char * elf = (const unsigned char *)p;

    ///////////////////////////////////////////////////////////

    close(fd);

    int ret;

    switch (a[4]) {
        case ELFCLASS32:
            if (swap == 0) {
                ret = handle_elf32(elf);
            } else {
                ret = handle_elf32_swap(elf);
            }
            break;
        case ELFCLASS64:
            if (swap == 0) {
                ret = handle_elf64(elf);
            } else {
                ret = handle_elf64_swap(elf);
            }
            break;
        default: 
            fprintf(stderr, "Invalid ELF file: %s\n", fp);
            ret = 10;
    }

    munmap(p, st.st_size);

    return ret;
}
