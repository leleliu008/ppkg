#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

// https://elixir.bootlin.com/musl/v1.2.5/source/include/dirent.h#L40
#define _BSD_SOURCE
#include <dirent.h>

#include <sys/stat.h>

#include <elf.h>

int handle_elf32(const int fd, const char * const fp) {
    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror(fp);
        close(fd);
        return 7;
    }

    ///////////////////////////////////////////////////////////

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

    Elf32_Phdr phdr;

    int hasPT_DYNAMIC = 0;

    for (unsigned int i = 1; i < ehdr.e_phnum; i++) {
        ret = pread(fd, &phdr, sizeof(Elf32_Phdr), ehdr.e_phoff + i * ehdr.e_phentsize);

        if (ret == -1) {
            perror(fp);
            return 17;
        }

        if ((size_t)ret != sizeof(Elf32_Phdr)) {
            perror(fp);
            fprintf(stderr, "not fully read.\n");
            return 18;
        }

        if (phdr.p_type == PT_DYNAMIC) {
            hasPT_DYNAMIC = 1;
            break;
        }
    }

    if (hasPT_DYNAMIC == 0) {
        return 0;
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

    int hasdynstrSection = 0;

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
            if (strcmp(shstrtab + shdr.sh_name, ".dynstr") == 0) {
                hasdynstrSection = 1;
                break;
            }
        }
    }

    if (hasdynstrSection == 0) {
        return 0;
    }

    ///////////////////////////////////////////////////////////

    Elf32_Dyn dyn;

    for (int i = 0; ; i++) {
        ret = pread(fd, &dyn, sizeof(Elf32_Dyn), phdr.p_offset + i * sizeof(Elf32_Dyn));

        if (ret == -1) {
            perror(fp);
            return 17;
        }

        if ((size_t)ret != sizeof(Elf32_Dyn)) {
            perror(fp);
            fprintf(stderr, "not fully read.\n");
            return 18;
        }

        if (dyn.d_tag == DT_NULL) {
            break;
        }

        if (dyn.d_tag == DT_FLAGS_1) {
            if (dyn.d_un.d_val & DF_1_PIE) {
                return 200;
            }
        }
    }

    return 0;
}

int handle_elf64(const int fd, const char * const fp) {
    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror(fp);
        close(fd);
        return 7;
    }

    ///////////////////////////////////////////////////////////

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

    Elf64_Phdr phdr;

    int hasPT_DYNAMIC = 0;

    for (unsigned int i = 1; i < ehdr.e_phnum; i++) {
        ret = pread(fd, &phdr, sizeof(Elf64_Phdr), ehdr.e_phoff + i * ehdr.e_phentsize);

        if (ret == -1) {
            perror(fp);
            return 17;
        }

        if ((size_t)ret != sizeof(Elf64_Phdr)) {
            perror(fp);
            fprintf(stderr, "not fully read.\n");
            return 18;
        }

        if (phdr.p_type == PT_DYNAMIC) {
            hasPT_DYNAMIC = 1;
            break;
        }
    }

    if (hasPT_DYNAMIC == 0) {
        return 0;
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

    int hasdynstrSection = 0;

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
            if (strcmp(shstrtab + shdr.sh_name, ".dynstr") == 0) {
                hasdynstrSection = 1;
                break;
            }
        }
    }

    if (hasdynstrSection == 0) {
        return 0;
    }

    ///////////////////////////////////////////////////////////

    Elf64_Dyn dyn;

    for (int i = 0; ; i++) {
        ret = pread(fd, &dyn, sizeof(Elf64_Dyn), phdr.p_offset + i * sizeof(Elf64_Dyn));

        if (ret == -1) {
            perror(fp);
            return 17;
        }

        if ((size_t)ret != sizeof(Elf64_Dyn)) {
            perror(fp);
            fprintf(stderr, "not fully read.\n");
            return 18;
        }

        if (dyn.d_tag == DT_NULL) {
            break;
        }

        if (dyn.d_tag == DT_FLAGS_1) {
            if (dyn.d_un.d_val & DF_1_PIE) {
                return 200;
            }
        }
    }

    return 0;
}

typedef int (*FN)(const char * fp);

static int determine(const char * fp) {
    struct stat st;

    if (stat(fp, &st) == -1) {
        perror(fp);
        return 100;
    }

    if (st.st_size < 52) {
        return 0;
    }

    int fd = open(fp, O_RDONLY);

    if (fd == -1) {
        perror(fp);
        return 101;
    }

    ///////////////////////////////////////////////////////////

    unsigned char a[32];

    ssize_t readBytes = read(fd, a, 32);

    if (readBytes == -1) {
        perror(fp);
        close(fd);
        return 102;
    }

    if (readBytes != 32) {
        perror(fp);
        close(fd);
        fprintf(stderr, "not fully read file: %s\nexpect: 32bytes\nactual: %ldbytes", fp, readBytes);
        return 103;
    }

    ///////////////////////////////////////////////////////////

    // https://www.sco.com/developers/gabi/latest/ch4.eheader.html
    if (a[0] == 0x7F && a[1] == 0x45 && a[2] == 0x4C && a[3] == 0x46) {
        unsigned short t;

        if (a[5] == ELFDATA2MSB) {
            unsigned char b[2] = { a[17], a[16] };
            memcpy(&t, b, sizeof(unsigned short));
        } else {
            memcpy(&t, a + 16, sizeof(unsigned short));
        }

        const char * s;

        switch (t) {
            case ET_REL:
                s = "REL";
                break;
            case ET_EXEC:
                s = "EXE";
                break;
            case ET_DYN:
                if (a[4] == ELFCLASS64) {
                    Elf64_Addr entry;

                    memcpy(&entry, a + 24, sizeof(Elf64_Addr));

                    // A shared library's e_entry is usually 0x0
                    if (entry == 0) {
                        s = "DSO";
                    } else {
                        int ret = handle_elf64(fd, fp);

                        if (ret == 0) {
                            s = "EXE";
                        } else if (ret == 200) {
                            s = "PIE";
                        } else {
                            close(fd);
                            return ret;
                        }
                    }

                    break;
                }

                if (a[4] == ELFCLASS32) {
                    Elf32_Addr entry;

                    memcpy(&entry, a + 24, sizeof(Elf32_Addr));

                    if (entry == 0) {
                        s = "DSO";
                    } else {
                        int ret = handle_elf32(fd, fp);

                        if (ret == 0) {
                            s = "EXE";
                        } else if (ret == 200) {
                            s = "PIE";
                        } else {
                            close(fd);
                            return ret;
                        }

                    }
                }

                break;
            default:
                s = "???";
                break;
        }

        printf("%hd %s %s\n", t, s, fp);
    }

    close(fd);
    return 0;
}

int scan(const char * dirPath) {
    DIR * dir = opendir(dirPath);

    if (dir == NULL) {
        perror(dirPath);
        return 104;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                return 0;
            } else {
                perror(dirPath);
                closedir(dir);
                return 105;
            }
        }

        //puts(dir_entry->d_name);

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        FN fn = NULL;

        switch (dir_entry->d_type) {
            case DT_DIR:
                fn = scan;
                break;
            case DT_REG:
                fn = determine;
                break;
        }

        if (fn != NULL) {
            char p[4096];

            int ret = snprintf(p, 4096, "%s/%s", dirPath, dir_entry->d_name);

            if (ret < 0) {
                perror(NULL);
                closedir(dir);
                return 106;
            }

            ret = fn(p);

            if (ret != 0) {
                closedir(dir);
                return ret;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        fprintf(stderr, "Usage: %s <DIR>\n", argv[0]);
        return 1;
    }

    if (argv[1][0] == '\0') {
        fprintf(stderr, "Usage: %s <DIR>, <DIR> must be non-empty.\n", argv[0]);
        return 2;
    }

    if (chdir(argv[1]) == -1) {
        perror(argv[1]);
        return 3;
    }

    return scan(".");
}
