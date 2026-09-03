#include <errno.h>
#include <stdio.h>

#include <unistd.h>
#include <dirent.h>

int main(int argc, char* argv[]) {
    if (argc == 1) {
        fprintf(stderr, "%s <DIR>\n", argv[0]);
        return 1;
    }

    if (argv[1][0] == '\0') {
        fprintf(stderr, "%s <DIR>, <DIR> must not be empty.\n", argv[0]);
        return 2;
    }

    const char * const dirpath = argv[1];

    DIR * dir = opendir(dirpath);

    if (dir == NULL) {
        perror(dirpath);
        return 3;
    }

loop:
    errno = 0;

    struct dirent * dir_entry = readdir(dir);

    if (dir_entry == NULL) {
        if (errno == 0) {
            closedir(dir);

            if (rmdir(dirpath) == -1) {
                perror(dirpath);
                return 4;
            } else {
                return 0;
            }
        } else {
            perror(dirpath);
            closedir(dir);
            return 5;
        }
    }

    const char * p = dir_entry->d_name;

    if (p[0] == '.') {
        if (p[1] == '\0') {
            goto loop;
        }

        if (p[1] == '.' && p[2] == '\0') {
            goto loop;
        }
    }

    closedir(dir);

    return 0;
}
