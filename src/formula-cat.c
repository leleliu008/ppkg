#include <stdio.h>
#include <stdlib.h>

#include "ppkg.h"

int ppkg_formula_cat(const char * packageName) {
    char * formulaFilePath = NULL;

    int ret = ppkg_formula_locate(packageName, &formulaFilePath);

    if (ret != PPKG_OK) {
        return ret;
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

    char   buff[1024];
    size_t size;

    for(;;) {
        size = fread(buff, 1, 1024, file);

        if (ferror(file)) {
            fclose(file);
            return PPKG_ERROR;
        }

        if (fwrite(buff, 1, size, stdout) != size || ferror(stdout)) {
            fclose(file);
            return PPKG_ERROR;
        }

        if (feof(file)) {
            fclose(file);
            return PPKG_OK;
        }
    }
}
