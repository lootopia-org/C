#pragma once

#include "errors.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#define STATIC_LIB_DEF static inline

#define GET_ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))
#define FOREACH(item, array) \
    for (size_t keep = 1, count = 0, size = sizeof(array)/sizeof(array[0]); \
         keep && count < size; \
         keep = !keep, count++) \
        for (item = &array[count]; keep; keep = !keep)
#define FOR_EACH_DIR_ENTRY(dir_path, entry_var) \
    DIR *_dir_##entry_var = opendir(dir_path); \
    if (!_dir_##entry_var) { \
        PRINT_ERROR_DO_ACTION("Failed to open directory: %s", {}, dir_path); \
    } else { \
        struct dirent *entry_var; \
        while ((entry_var = readdir(_dir_##entry_var)) != NULL) { \
            if (compare_string(entry_var->d_name, ".") || \
                compare_string(entry_var->d_name, "..")) continue;
#define CLOSE_DIR(entry_var) \
    } \
    closedir(_dir_##entry_var); \
    }

STATIC_LIB_DEF char* concat_and_ret_str(const char *s1, const char *s2) {
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    char *res = (char *)malloc(len1 + len2 + 1);
    if(res) {
        memcpy(res, s1, len1);
        memcpy(res + len1, s2, len2 + 1); 
    }
    return res;
}

STATIC_LIB_DEF int compare_string(char *s1, char *s2){
  if(strcmp(s1, s2) == 0)
    return 1;

  return 0;
}

STATIC_LIB_DEF char* trim_str(char* str) {
    while (*str == ' ' || *str == '\t') str++;
    
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\r')) {
        *end = '\0';
        end--;
    }
    
    return str;
}
