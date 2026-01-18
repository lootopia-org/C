#pragma once

#include "./arguments.h"
#include<stdlib.h>

typedef enum{
  INT_T,
  STR_T
} TokenType;

typedef struct {
    const char *key;          
    size_t offset;               
    TokenType type;
} ConfigEntry;


void *load_config(void *config, const ConfigEntry *entries, size_t entry_count);

void free_config(void *config, const ConfigEntry *entries, size_t entry_count);
