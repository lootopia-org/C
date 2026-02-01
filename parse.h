#pragma once
#include <stdlib.h>
#include <string.h>
#include "errors.h"
#include "macros.h"
#include "config.h"

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
        ERROR_EXIT(FIND_FILE_ERROR, filename);
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

STATIC_LIB_DEF char* get_env_value(dotenv_array_t *env_array, const char *key) {
    for (size_t i = 0; i < env_array->count; i++) {
        if (strcmp(env_array->items[i].name, key) == 0) {
            return env_array->items[i].value;
        }
    }
    return NULL;
}

STATIC_LIB_DEF char* substitute_env_vars(const char *value, dotenv_array_t *env_array) {
    if (!value || strlen(value) < 3) return strdup(value);
    
    if (value[0] == '$' && value[1] == '{') {
        const char *end = strchr(value + 2, '}');
        if (!end) {
            fprintf(stderr, "Error: Malformed environment variable reference: %s\n", value);
            exit(1);
        }
        
        size_t var_len = end - (value + 2);
        char *var_name = (char*)malloc(var_len + 1);
        strncpy(var_name, value + 2, var_len);
        var_name[var_len] = '\0';
        
        char *env_value = get_env_value(env_array, var_name);
        if (!env_value) {
            fprintf(stderr, "Error: Environment variable '%s' not found in .env file\n", var_name);
            free(var_name);
            exit(1);
        }
        
        char *result = strdup(env_value);
        free(var_name);
        return result;
    }
    
    return strdup(value);
}

STATIC_LIB_DEF YamlArray* parse_yaml(const char* yaml) {
    dotenv_array_t *env_array = read_dot_env();
    
    const char *ptr = yaml;
    char line[1024];
    int capacity = 10;
    int count = 0;
    
    Yaml *entries = (Yaml *)malloc(sizeof(Yaml) * capacity);
    if (!entries) {
        destroy_dotenv_array(env_array);
        return NULL;
    }
    
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
            
            char *value_copy = substitute_env_vars(*value ? value : "", env_array);
            
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
    
    destroy_dotenv_array(env_array);
    
    YamlArray *result = (YamlArray*)malloc(sizeof(YamlArray));
    result->ymal = entries;
    result->count = count;
    result->capacity = capacity;
    
    return result;
}

STATIC_LIB_DEF char* parse_yaml_value(YamlArray *conf, const char *key) {
    if (!conf || !key) return NULL;
    
    for (int i = 0; i < conf->count; i++) {
        if (conf->ymal[i].key && strcmp(conf->ymal[i].key, key) == 0) {
            return conf->ymal[i].value;
        }
    }
    return NULL;
}

STATIC_LIB_DEF void free_yaml(YamlArray* arr) {
    for (int i = 0; i < arr->count; i++) {
        free(arr->ymal[i].key);
        free(arr->ymal[i].value);
        free(arr->ymal[i].item);
    }
    free(arr->ymal);
    free(arr);
}
