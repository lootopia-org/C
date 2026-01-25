#pragma once

#include <stdlib.h>

#include "errors.h"
#include "macros.h"


typedef struct {
    char *key;
    char *value;
    char *item;
} Yaml;

typedef struct {
  Yaml *ymal;
  int count;
  int capacity;
} YamlArray;

STATIC_LIB_DEF char* parse_file(const char *filename) {
    FILE *fh = fopen(filename, "r");
    if (!fh) 
      ERROR_EXIT("%s", FIND_FILE_ERROR);

    fseek(fh, 0, SEEK_END);
    long filesize = ftell(fh);
    fseek(fh, 0, SEEK_SET);

    char *buffer = (char *)malloc(filesize + 1);
    if (!buffer) 
      PRINT_ERROR_DO_ACTION(ALLOCATION_ERROR, {fclose(fh); exit(1);}, "buffer");


    fread(buffer, 1, filesize, fh);
    buffer[filesize] = '\0';
    
    fclose(fh);
    return buffer;
}

STATIC_LIB_DEF YamlArray* parse_yaml(const char* yaml) {
    const char *ptr = yaml;
    char line[1024];
    int capacity = 10;
    int count = 0;
    
    Yaml *entries = (Yaml *)malloc(sizeof(Yaml) * capacity);
    if (!entries) return NULL;
    
    char *current_key = NULL;
    
    while (*ptr) {
        int i = 0;
        while (*ptr && *ptr != '\n' && i < sizeof(line)-1) {
            line[i++] = *ptr++;
        }
        line[i] = '\0';
        if (*ptr == '\n') ptr++;
        
        char working_line[1024];
        strcpy(working_line, line);
        char *trimmed = trim_str(working_line);
        
        if (*trimmed == '\0') continue;
        
        if (trimmed[0] == '-' && trimmed[1] == ' ') {
            char *item = strdup(trimmed + 2);
            if (!item) continue;
            
            if (count >= capacity) {
                capacity *= 2;
                Yaml *new_entries = (Yaml *)realloc(entries, sizeof(Yaml) * capacity);
                if (!new_entries) {
                    free(item);
                    continue;
                }
                entries = new_entries;
            }
            
            entries[count].key = current_key ? strdup(current_key) : NULL;
            entries[count].value = NULL;
            entries[count].item = item;
            count++;
            continue;
        }
        
        char *colon = strchr(trimmed, ':');
        if (colon) {
            *colon = '\0';
            char *key = trim_str(trimmed);
            char *value = trim_str(colon + 1);
            
            char *key_copy = strdup(key);
            char *value_copy = strdup(*value ? value : "");
            
            if (!key_copy || !value_copy) {
                free(key_copy);
                free(value_copy);
                continue;
            }
            
            if (current_key) free(current_key);
            current_key = strdup(key_copy);
            
            if (count >= capacity) {
                capacity *= 2;
                Yaml *new_entries = (Yaml*)realloc(entries, sizeof(Yaml) * capacity);
                if (!new_entries) {
                    free(key_copy);
                    free(value_copy);
                    continue;
                }
                entries = new_entries;
            }
            
            entries[count].key = key_copy;
            entries[count].value = value_copy;
            entries[count].item = NULL;
            count++;
        }
    }
    
    if (current_key) free(current_key);
    
    YamlArray *result = (YamlArray*)malloc(sizeof(YamlArray));
    
    result->ymal = entries;
    result->count = count;
    result->capacity = capacity;
    
    return result;
}

STATIC_LIB_DEF void free_yaml(YamlArray* arr) {
    for (int i = 0; i < arr->count; i++) {
        free(arr->ymal[i].key);
        free(arr->ymal[i].value);
        free(arr->ymal[i].item);
    }
    free(arr);
}
