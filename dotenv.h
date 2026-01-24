#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "./arguments.h"
#include "./dotenv.h"
#include "./errors.h"
#include "macros.h"

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


dotenv_array_t *read_dot_env(EMPTY);
void destroy_dotenv_array(IN dotenv_array_t *array);

#ifdef DOTENV
  dotenv_array_t *read_dot_env(EMPTY) {
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

  void destroy_dotenv_array(IN dotenv_array_t *array) {
      if (!array) return;
      for (size_t i = 0; i < array->count; i++) {
          free(array->items[i].name);
          free(array->items[i].value);
      }
      free(array->items);
      free(array);
  }
#endif // DOTENV
