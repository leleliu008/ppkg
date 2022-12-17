#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "core/fs.h"
#include "ppkg.h"

int ppkg_formula_bat(const char * packageName) {
    char * formulaFilePath = NULL;

    int resultCode = ppkg_formula_path(packageName, &formulaFilePath);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    resultCode = execlp("bat", "bat", "--paging=never", formulaFilePath, NULL);

    if (resultCode == -1) {
        fprintf(stderr, "command not found: bat\n");
        resultCode = PPKG_ERROR;
    }

    free(formulaFilePath);

    return resultCode;
}
