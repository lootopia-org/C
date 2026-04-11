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

#define OBJ_PAIR(k, v)                                                         \
  {.key = (k),                                                                 \
   .value = (v),                                                               \
   .key_len = sizeof(k) - 1,                                                   \
   .value_len = sizeof(v) - 1}

#define INIT_OBJS_1(k1, v1) OBJ_PAIR(k1, v1)

#define INIT_OBJS_2(k1, v1, k2, v2) OBJ_PAIR(k1, v1), OBJ_PAIR(k2, v2)

#define INIT_OBJS_3(k1, v1, k2, v2, k3, v3)                                    \
  OBJ_PAIR(k1, v1), OBJ_PAIR(k2, v2), OBJ_PAIR(k3, v3)

#define OBJ_GET(arr, key) obj_get((arr), sizeof(arr) / sizeof((arr)[0]), (key))

#define OBJ_PUT(list, k, v)                                                    \
  do {                                                                         \
    int found = 0;                                                             \
    for (size_t i = 0; i < (list)->count; i++) {                               \
      if (strcmp((list)->items[i].key, (k)) == 0) {                            \
        (list)->items[i].value = (v);                                          \
        (list)->items[i].value_len = sizeof(v) - 1;                            \
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
                                               .key_len = sizeof(k) - 1,       \
                                               .value_len = sizeof(v) - 1};    \
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

#define GET_MACRO(_1, _2, _3, _4, _5, _6, NAME, ...) NAME

#define INIT_OBJS(...)                                                         \
  GET_MACRO(__VA_ARGS__, INIT_OBJS_3, INIT_OBJS_2, INIT_OBJS_1)(__VA_ARGS__)
