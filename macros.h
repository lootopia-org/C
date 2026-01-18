#pragma once

#include <stdlib.h>
#include <string.h>
#define GET_ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

static inline char* concat_and_ret_str(const char *s1, const char *s2) {
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    char *res = (char *)malloc(len1 + len2 + 1);
    if(res) {
        memcpy(res, s1, len1);
        memcpy(res + len1, s2, len2 + 1); 
    }
    return res;
}                        
