#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <elf.h>

int main(int argc, char* argv[]) {
    if (argc == 1) {
        fprintf(stderr, "Usage: %s <FILE>\n", argv[0]);
        return 1;
    }

    const char * const fp = argv[1];

    if (fp[0] == '\0') {
        fprintf(stderr, "Usage: %s <FILE>, <FILE> must be non-empty.\n", argv[0]);
        return 2;
    }

    ///////////////////////////////////////////////////////////

    struct stat st;

    if (stat(fp, &st) == -1) {
        perror(fp);
        return 100;
    }

    if (st.st_size == 0) {
        return 0;
    }

    ///////////////////////////////////////////////////////////

    int n;

    if (st.st_size < 18) {
        n = st.st_size;
    } else {
        n = 18;
    }

    ///////////////////////////////////////////////////////////

    int fd = open(fp, O_RDONLY);

    if (fd == -1) {
        perror(fp);
        return 101;
    }

    ///////////////////////////////////////////////////////////

    unsigned char a[n];

    ssize_t readBytes = read(fd, a, n);

    if (readBytes == -1) {
        perror(fp);
        close(fd);
        return 102;
    }

    if (readBytes != n) {
        perror(fp);
        close(fd);
        fprintf(stderr, "not fully read file: %s\nexpect: %dbytes\nactual: %ldbytes", fp, n, readBytes);
        return 103;
    }

    ///////////////////////////////////////////////////////////

    // https://www.sco.com/developers/gabi/latest/ch4.eheader.html
    if (n == 18 && a[0] == 0x7F && a[1] == 0x45 && a[2] == 0x4C && a[3] == 0x46) {
        if (a[5] == ELFDATA2MSB) {
            a[4] = a[17];
            a[5] = a[16];
        } else {
            a[4] = a[16];
            a[5] = a[17];
        }
    }

    close(fd);

    const char * const table = "0123456789ABCDEF";

    n = (n > 6) ? 6 : n;

    int m = n << 1;

    char b[m + 1];

    b[m] = '\0';

    for (int i = 0; i < n; i++) {
        int j = i << 1;
        b[j]     = table[a[i] >> 4];
        b[j + 1] = table[a[i] & 0x0F];
    }

    puts(b);

    return 0;
}
