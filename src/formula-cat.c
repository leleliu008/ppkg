#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/fs.h"
#include "ppkg.h"

int ppkg_formula_cat(const char * packageName) {
    char * formulaFilePath = NULL;

    int resultCode = ppkg_formula_path(packageName, &formulaFilePath);

    if (resultCode != PPKG_OK) {
        return resultCode;
    }

    FILE * file = fopen(formulaFilePath, "r");

    if (file == NULL) {
        perror(formulaFilePath);
        free(formulaFilePath);
        return PPKG_ERROR;
    }

    printf("formula: %s\n", formulaFilePath);

    free(formulaFilePath);
    formulaFilePath = NULL;

    char buff[1024];
    int  size = 0;
    while((size = fread(buff, 1, 1024, file)) != 0) {
        fwrite(buff, 1, size, stdout);
    }

    fclose(file);

    return PPKG_OK;
}
