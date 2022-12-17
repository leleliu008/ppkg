#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>

bool regex_matched(const char * content, const char * pattern) {
    //printf("content = %s\n", content);
    //printf("pattern = %s\n", pattern);
    if (content == NULL || pattern == NULL) {
        return false;
    }

    regex_t regex;

    if (regcomp(&regex, pattern, REG_EXTENDED) == 0) {
        regmatch_t regmatch[2];
        if (regexec(&regex, content, 2, regmatch, 0) == 0) {
            //printf("regmatch[0].rm_so=%d\n", regmatch[0].rm_so);
            //printf("regmatch[0].rm_eo=%d\n", regmatch[0].rm_eo);
            if (regmatch[0].rm_so >= 0 && regmatch[0].rm_eo > regmatch[0].rm_so) {
                regfree(&regex);
                return true;
            }
        }
    }

    regfree(&regex);
    return false;
}

char* regex_extract(const char * content, const char * pattern) {
    //printf("content = %s\n", content);
    //printf("pattern = %s\n", pattern);
    if (content == NULL || pattern == NULL) {
        return NULL;
    }

    regex_t regex;

    if (regcomp(&regex, pattern, REG_EXTENDED) == 0) {
        regmatch_t regmatch[2];
        if (regexec(&regex, content, 2, regmatch, 0) == 0) {
            //printf("regmatch[0].rm_so=%d\n", regmatch[0].rm_so);
            //printf("regmatch[0].rm_eo=%d\n", regmatch[0].rm_eo);
            if (regmatch[0].rm_so >= 0 && regmatch[0].rm_eo > regmatch[0].rm_so) {
                int n = regmatch[0].rm_eo - regmatch[0].rm_so;
                const char * str = &content[regmatch[0].rm_so];
                char * result = (char*)calloc(n+1, sizeof(char));
                strncpy(result, str, n);
                regfree(&regex);
                return result;
            }
        }
    }

    regfree(&regex);
    return NULL;
}
