#include <errno.h>
#include <stdio.h>

#include <unistd.h>
#include <dirent.h>

/**
 *  check if the given path is an empty dir
 *  error occurs, -1 is returned and errno is set to indicate the error
 *  if a empty dir, 0 is returned
 *  not a empty dir, 1 is returned
 */
static int is_empty_dir(const char * const dirpath) {
    DIR * dir = opendir(dirpath);

    if (dir == NULL) {
        return -1;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return 0;
            } else {
                int err = errno;
                closedir(dir);
                errno = err;
                return -1;
            }
        }

        const char * const p = dir_entry->d_name;

        if (p[0] == '.') {
            if (p[1] == '\0') continue;
            if (p[1] == '.') {
                if (p[2] == '\0') continue;
            }
        }

        closedir(dir);

        return 1;
    }
}

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

    int ret = is_empty_dir(dirpath);

    if (ret == -1) {
        perror(dirpath);
        return 3;
    }

    if (ret == 0) {
        if (rmdir(dirpath) == -1) {
            perror(dirpath);
            return 4;
        }
    }

    return 0;
}
