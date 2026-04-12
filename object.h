#pragma once
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *key;
  char *value;
  size_t key_len;
  size_t value_len;
} obj_t;

typedef struct {
  obj_t *items;
  size_t count;
  size_t capacity;
} obj_list_t;

#define OBJ_PAIR(k, v)                                                         \
  {.key = (k),                                                                 \
   .value = (v),                                                               \
   .key_len = sizeof(k) - 1,                                                   \
   .value_len = sizeof(v) - 1}

#define OBJ_LIST(...)                                                          \
  {.items = (obj_t[]){__VA_ARGS__},                                            \
   .count = sizeof((obj_t[]){__VA_ARGS__}) / sizeof(obj_t),                    \
   .capacity = sizeof((obj_t[]){__VA_ARGS__}) / sizeof(obj_t)}

#define OBJ_GET(list, k) obj_get((list)->items, (list)->count, (k))

#define OBJ_PUT(list, k, v)                                                    \
  do {                                                                         \
    int found = 0;                                                             \
    for (size_t i = 0; i < (list)->count; i++) {                               \
      if (strcmp((list)->items[i].key, (k)) == 0) {                            \
        (list)->items[i].value = (v);                                          \
        (list)->items[i].value_len = strlen(v);                                \
        found = 1;                                                             \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    if (!found) {                                                              \
      if ((list)->count == (list)->capacity) {                                 \
        (list)->capacity = (list)->capacity ? (list)->capacity * 2 : 4;        \
        (list)->items =                                                        \
            realloc((list)->items, (list)->capacity * sizeof(obj_t));          \
      }                                                                        \
      (list)->items[(list)->count++] = (obj_t){.key = (k),                     \
                                               .value = (v),                   \
                                               .key_len = strlen(k),           \
                                               .value_len = strlen(v)};        \
    }                                                                          \
  } while (0)

#define OBJ_REMOVE(list, k)                                                    \
  do {                                                                         \
    for (size_t i = 0; i < (list)->count; i++) {                               \
      if (strcmp((list)->items[i].key, (k)) == 0) {                            \
        (list)->items[i] = (list)->items[(list)->count - 1];                   \
        (list)->count--;                                                       \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
  } while (0)
