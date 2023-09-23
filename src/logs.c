#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "ppkg.h"

int ppkg_logs(const char * packageName) {
    int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    char   ppkgHomeDIR[256];
    size_t ppkgHomeDIRLength;

    ret = ppkg_home_dir(ppkgHomeDIR, 256, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t   metaInfoDIRLength = ppkgHomeDIRLength + strlen(packageName) + 18U;
    char     metaInfoDIR[metaInfoDIRLength];
    snprintf(metaInfoDIR, metaInfoDIRLength, "%s/installed/%s/.ppkg", ppkgHomeDIR, packageName);

    size_t   receiptFilePathLength = metaInfoDIRLength + 13U;
    char     receiptFilePath[receiptFilePathLength];
    snprintf(receiptFilePath, receiptFilePathLength, "%s/RECEIPT.yml", metaInfoDIR);

    struct stat st;

    if (stat(receiptFilePath, &st) != 0 || !S_ISREG(st.st_mode)) {
        return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    DIR * dir = opendir(metaInfoDIR);

    if (dir == NULL) {
        perror(metaInfoDIR);
        return PPKG_ERROR;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return PPKG_OK;
            } else {
                perror(metaInfoDIR);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        //puts(dir_entry->d_name);

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t   filepathLength = metaInfoDIRLength + strlen(dir_entry->d_name) + 2U;
        char     filepath[filepathLength];
        snprintf(filepath, filepathLength, "%s/%s", metaInfoDIR, dir_entry->d_name);

        if (stat(filepath, &st) == 0 && S_ISDIR(st.st_mode)) {
            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror(NULL);
            closedir(dir);
            return PPKG_ERROR;
        }

        if (pid == 0) {
            execlp("bat", "bat", filepath, NULL);
            perror("bat");
            exit(255);
        } else {
            int childProcessExitStatusCode;

            if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
                perror(NULL);
                closedir(dir);
                return PPKG_ERROR;
            }

            if (childProcessExitStatusCode != 0) {
                if (WIFEXITED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running command 'bat %s' exit with status code: %d\n", filepath, WEXITSTATUS(childProcessExitStatusCode));
                } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running command 'bat %s' killed by signal: %d\n", filepath, WTERMSIG(childProcessExitStatusCode));
                } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running command 'bat %s' stopped by signal: %d\n", filepath, WSTOPSIG(childProcessExitStatusCode));
                }

                closedir(dir);
                return PPKG_ERROR;
            }
        }
    }
}
