#pragma once

#include<stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./arguments.h"
#include "./config.h"
#include "./errors.h"
#include "./macros.h"

#define DOTENV_FILE ".env"
#define PARENT_DIR  "../"
#define READ_PERMISSIONS "r"

#define LINE_SIZE 1024
#define INIT_CAP 10
#define INIT_COUNT 0
#define INCREASE_CAP 2

typedef struct DOTENV {
    char *name;
    char *value;
} dotenv_t;

typedef struct DOTENV_ARRAY {
    dotenv_t *items;
    size_t count;
} dotenv_array_t;

typedef enum{
  INT_T,
  STR_T
} TokenType;

typedef struct {
    const char *key;          
    size_t offset;               
    TokenType type;
} ConfigEntry;


void *load_config(const ConfigEntry *entries, size_t entry_count, size_t struct_size);
void free_config(void *config, const ConfigEntry *entries, size_t entry_count);

#ifdef CONFIG_IMPLEMENTATION
  static dotenv_array_t *read_dot_env(EMPTY) {
    char *equals_sign, *name, *value;
    char line[LINE_SIZE];
    size_t count = INIT_COUNT;
    size_t cap = INIT_CAP; 
    dotenv_t *items = malloc(cap * sizeof(dotenv_t));
    dotenv_array_t *result = malloc(sizeof(dotenv_array_t));
    FILE *file = fopen(DOTENV_FILE, READ_PERMISSIONS);
    if(!file){
      char *path = concat_and_ret_str(PARENT_DIR, DOTENV_FILE); 
      file = fopen(path, READ_PERMISSIONS);
      if(!file)
        ERROR_EXIT("Error reading dotenv file, make sure it exists");
    }
    
    if (!items) 
      ERROR_EXIT(ALLOCATION_ERROR, "items");
    if (!result) 
      ERROR_EXIT("Memory allocation failed for result struct");

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0; 

        if (line[0] == '\0' || line[0] == '#') 
          continue; 

        equals_sign = strchr(line, '=');
        
        if (!equals_sign) 
          continue;

        *equals_sign = '\0';
        name = line;
        value = equals_sign + 1;

        if (count >= cap) {
            cap *= INCREASE_CAP;
            items = realloc(items, cap * sizeof(dotenv_t));
            if (!items) 
              ERROR_EXIT("Memory reallocation failed for items");
        }

        items[count].name = strdup(name);
        items[count].value = strdup(value);
        count++;
    }

    fclose(file);


    result->items = items;
    result->count = count;
    return result;
  }

  static void destroy_dotenv_array(IN dotenv_array_t *array) {
      if (!array) return;
      for (size_t i = 0; i < array->count; i++) {
          free(array->items[i].name);
          free(array->items[i].value);
      }
      free(array->items);
      free(array);
  }

  void *load_config(const ConfigEntry *entries, size_t entry_count, size_t struct_size) {
      int found;
      size_t i, j;
      dotenv_array_t *env = read_dot_env();
      void *config = malloc(struct_size);
      if (!config) 
          ERROR_EXIT(ALLOCATION_ERROR, "config");

      memset(config, 0, struct_size); 
      
      for (i = 0; i < entry_count; i++) {
          found = 0;
          for (j = 0; j < env->count; j++) {
              if (strcmp(env->items[j].name, entries[i].key) == 0) {
                  found = 1;
                  void *field_ptr = (char *)config + entries[i].offset;
                  
                  if (entries[i].type == INT_T) {
                      *(int *)field_ptr = atoi(env->items[j].value);
                  } else if (entries[i].type == STR_T) {
                      *(char **)field_ptr = strdup(env->items[j].value);
                  }
                  break;
              }
          }
          if (!found) {
              ERROR_EXIT(CONFIG_ERROR, entries[i].key);
          }
      }
      destroy_dotenv_array(env);
      return config;
  }

  void free_config(void *config, const ConfigEntry *entries, size_t entry_count) {
      if (!config) return;
      
      for (size_t i = 0; i < entry_count; i++) {
          if (entries[i].type == STR_T) {
              char **str_ptr = (char **)((char *)config + entries[i].offset);
              if (*str_ptr) {
                  free(*str_ptr);
                  *str_ptr = NULL;
              }
          }
      }
      free(config);
  }
#endif // CONFIG
