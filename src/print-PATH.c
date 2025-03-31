#include<stdio.h>
#include<stdlib.h>

#include<unistd.h>
#include<limits.h>

void listPATH() {
    const char * p = getenv("PATH");

    char buf[PATH_MAX];

    while (p != NULL) {
        for (int i = 0;; i++) {
            if (p[i] == '\0') {
                p = NULL;

                buf[i] = '\0';

                if (i != 0) {
                    puts(buf);
                }

                break;
            } else if (p[i] == ':') {
                p += i + 1;

                buf[i] = '\0';

                if (i != 0) {
                    puts(buf);
                }

                break;
            } else {
                buf[i] = p[i];
            }
        }
    }
}
