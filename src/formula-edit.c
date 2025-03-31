#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>

#include "ppkg.h"

int ppkg_formula_edit(const char * packageName, const char * targetPlatformName, const char * editor) {
    char formulaFilePath[PATH_MAX];

    int ret = ppkg_formula_path(packageName, targetPlatformName, formulaFilePath);

    if (ret != PPKG_OK) {
        return ret;
    }

    if (editor == NULL || editor[0] == '\0') {
        editor = "vim";
    }

    if (editor[0] == '/') {
        execl (editor, editor, formulaFilePath, NULL);
    } else {
        execlp(editor, editor, formulaFilePath, NULL);
    }

    perror(editor);

    return PPKG_ERROR;
}
