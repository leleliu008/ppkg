#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(int argc, const char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <N> <ELF-FILEPATH>\n", argv[0]);
        return 1;
    }

    if (argv[1][0] == '\0') {
        printf("Usage: %s <N> <ELF-FILEPATH>, <N> must not be enpty.\n", argv[0]);
        return 2;
    }

    for (int i = 0; argv[1][i] != '\0'; i++) {
        if (argv[1][i] < '0' || argv[1][i] > '9') {
            printf("Usage: %s <N> <ELF-FILEPATH>, <N> must be an integer.\n", argv[0]);
            return 3;
        }
    }

    int n = atoi(argv[1]);

    const char * const fp = argv[2];

    if (fp[0] == '\0') {
        printf("Usage: %s <N> <ELF-FILEPATH>, <ELF-FILEPATH> must not be empty.\n", argv[0]);
        return 4;
    }

    int fd = open(fp, O_RDONLY);

    if (fd == -1) {
        perror(fp);
        return 5;
    }

    struct stat st;

    if (fstat(fd, &st) == -1) {
        perror(fp);
        close(fd);
        return 6;
    }

    if (st.st_size == 0) {
        close(fd);
        return 0;
    }

    if (st.st_size < n) {
        n = st.st_size;
    }

    ///////////////////////////////////////////////////////////

    unsigned char a[n];

    ssize_t readBytes = read(fd, a, n);

    if (readBytes == -1) {
        perror(fp);
        close(fd);
        return 7;
    }

    if (readBytes == n) {
        close(fd);

        const char * const table = "0123456789ABCDEF";

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
    } else {
        perror(fp);
        close(fd);
        fprintf(stderr, "not fully read file: %s\nexpect: %dbytes\nactual: %ldbytes", fp, n, readBytes);
        return 8;
    }
}
