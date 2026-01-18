#pragma once

#include "./arguments.h"
#include <stddef.h>

typedef struct {
    const char *key;          
    void *dest;               
    enum { INT_T, STR_T } type;
} config_entry_t;


void *load_config(config_entry_t *entries, size_t entry_count);

void free_config(IN config_entry_t *entries, IN size_t entry_count, IN void *config);
