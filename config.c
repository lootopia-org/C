#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./config.h"
#include "./dotenv.h"
#include "./errors.h"

void *load_config(config_entry_t *entries, size_t entry_count) {
    int found;
    size_t i, j;
    dotenv_array_t *env = read_dot_env();
    void *config = malloc(sizeof(void *));
    if (!config) 
      ERROR_EXIT(ALLOCATION_ERROR, "config");
    
    memset(config, 0, sizeof(void*));

    for (i = 0; i < entry_count; i++) {
        found = 0;
        for (j = 0; j < env->count; j++) {
            if (strcmp(env->items[j].name, entries[i].key) == 0) {
                found = 1;
                if (entries[i].type == INT_T) {
                    *(int *)entries[i].dest = atoi(env->items[j].value);
                } else if (entries[i].type == STR_T) {
                    *(char **)entries[i].dest = strdup(env->items[j].value);
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

void free_config(config_entry_t *entries, size_t entry_count, void *config) {
    if (!config) return;

    for (size_t i = 0; i < entry_count; i++) {
        if (entries[i].type == STR_T) {
            char **str_ptr = (char **)entries[i].dest;
            if (*str_ptr) {
                free(*str_ptr);
                *str_ptr = NULL;
            }
        }
    }

    free(config);
}

