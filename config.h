#pragma once

#include<stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./arguments.h"
#include "./config.h"
#include "./dotenv.h"
#include "./errors.h"


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
  #define DOTENV_IMPLEMENTATION
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
