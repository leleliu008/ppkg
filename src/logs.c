#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "ppkg.h"

int ppkg_logs(const char * packageName, const PPKGTargetPlatform * targetPlatform) {
    int ret = ppkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != PPKG_OK) {
        return ret;
    }

    if (targetPlatform == NULL) {
        return PPKG_ERROR_ARG_IS_NULL;
    }

    char   ppkgHomeDIR[PATH_MAX];
    size_t ppkgHomeDIRLength;

    ret = ppkg_home_dir(ppkgHomeDIR, &ppkgHomeDIRLength);

    if (ret != PPKG_OK) {
        return ret;
    }

    size_t metaInfoDIRCapacity = ppkgHomeDIRLength + targetPlatform->nameLen + targetPlatform->versLen + targetPlatform->archLen + strlen(packageName) + sizeof(PPKG_METADATA_DIR_PATH_RELATIVE_TO_INSTALLED_ROOT) + 15U;
    char   metaInfoDIR[metaInfoDIRCapacity];

    ret = snprintf(metaInfoDIR, metaInfoDIRCapacity, "%s/installed/%s-%s-%s/%s%s", ppkgHomeDIR, targetPlatform->name, targetPlatform->vers, targetPlatform->arch, packageName, PPKG_METADATA_DIR_PATH_RELATIVE_TO_INSTALLED_ROOT);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (chdir(metaInfoDIR) == -1) {
        if (errno == ENOENT) {
            return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
        } else {
            perror(metaInfoDIR);
            return PPKG_ERROR;
        }
    }

    struct stat st;

    if (stat("RECEIPT.yml", &st) != 0 || !S_ISREG(st.st_mode)) {
        return PPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * const str = "/uppm/installed/fzf/bin/fzf";

    size_t fzfCommandPathCapacity = ppkgHomeDIRLength + strlen(str) + sizeof(char);
    char   fzfCommandPath[fzfCommandPathCapacity];

    ret = snprintf(fzfCommandPath, fzfCommandPathCapacity, "%s%s", ppkgHomeDIR, str);

    if (ret < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return PPKG_ERROR;
    }

    if (pid == 0) {
        execlp(fzfCommandPath, fzfCommandPath, "--preview=bat --color=always --theme=Dracula {}", "--preview-window=right:75%", NULL);
        perror(fzfCommandPath);
        exit(255);
    } else {
        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return PPKG_ERROR;
        }

        if (childProcessExitStatusCode == 0) {
            return PPKG_OK;
        } else {
            if (WIFEXITED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command '%s' exit with status code: %d\n", fzfCommandPath, WEXITSTATUS(childProcessExitStatusCode));
            } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command '%s' killed by signal: %d\n", fzfCommandPath, WTERMSIG(childProcessExitStatusCode));
            } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command '%s' stopped by signal: %d\n", fzfCommandPath, WSTOPSIG(childProcessExitStatusCode));
            }

            return PPKG_ERROR;
        }
    }
}

static int xx(const char * metaInfoDIR, const size_t metaInfoDIRCapacity) {
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

        size_t filepathCapacity = metaInfoDIRCapacity + strlen(dir_entry->d_name) + 2U;
        char   filepath[filepathCapacity];

        int ret = snprintf(filepath, filepathCapacity, "%s/%s", metaInfoDIR, dir_entry->d_name);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            return PPKG_ERROR;
        }

        struct stat st;

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
