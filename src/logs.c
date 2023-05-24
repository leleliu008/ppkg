#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "ppkg.h"

int ppkg_logs(const char * packageName) {
    int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    char   ppkgHomeDir[256];
    size_t ppkgHomeDirLength;

    ret = ppkg_home_dir(ppkgHomeDir, 256, &ppkgHomeDirLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t   packageMetadataDirLength = ppkgHomeDirLength + strlen(packageName) + 18U;
    char     packageMetadataDir[packageMetadataDirLength];
    snprintf(packageMetadataDir, packageMetadataDirLength, "%s/installed/%s/.ppkg", ppkgHomeDir, packageName);

    size_t   receiptFilePathLength = packageMetadataDirLength + 13U;
    char     receiptFilePath[receiptFilePathLength];
    snprintf(receiptFilePath, receiptFilePathLength, "%s/receipt.yml", packageMetadataDir);

    struct stat st;

    if (stat(receiptFilePath, &st) != 0 || !S_ISREG(st.st_mode)) {
        return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    DIR * dir = opendir(packageMetadataDir);

    if (dir == NULL) {
        perror(packageMetadataDir);
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
                perror(packageMetadataDir);
                closedir(dir);
                return PPKG_ERROR;
            }
        }

        //puts(dir_entry->d_name);

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t   filepathLength = packageMetadataDirLength + strlen(dir_entry->d_name) + 2U;
        char     filepath[filepathLength];
        snprintf(filepath, filepathLength, "%s/%s", packageMetadataDir, dir_entry->d_name);

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
            char* argv[3] = { (char*)"bat", filepath, NULL };
            execvp(argv[0], argv);
            perror(argv[0]);
            exit(127);
        } else {
            int childProcessExitStatusCode;

            if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
                perror(NULL);
                closedir(dir);
                return PPKG_ERROR;
            }

            if (childProcessExitStatusCode != 0) {
                size_t   cmdLength = packageMetadataDirLength + filepathLength + 4U;
                char     cmd[cmdLength];
                snprintf(cmd, cmdLength, "bat %s", filepath);

                if (WIFEXITED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running command '%s' exit with status code: %d\n", cmd, WEXITSTATUS(childProcessExitStatusCode));
                } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running command '%s' killed by signal: %d\n", cmd, WTERMSIG(childProcessExitStatusCode));
                } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running command '%s' stopped by signal: %d\n", cmd, WSTOPSIG(childProcessExitStatusCode));
                }

                closedir(dir);
                return PPKG_ERROR;
            }
        }
    }
}
