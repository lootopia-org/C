#pragma once
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct obj_val_t obj_val_t;
typedef struct obj_t obj_t;

typedef enum {
  OBJ_STRING,
  OBJ_INT,
  OBJ_FLOAT,
  OBJ_BOOL,
  OBJ_ARRAY,
  OBJ_OBJECT
} obj_type_t;

struct obj_val_t {
  obj_type_t type;
  union {
    struct {
      char *ptr;
      size_t len;
    } string;
    int integer;
    float floating;
    int boolean;
    struct {
      obj_val_t *items;
      size_t count;
    } array;
    struct {
      obj_t *items;
      size_t count;
    } object;
  };
};

struct obj_t {
  char *key;
  size_t key_len;
  obj_val_t value;
};

typedef struct {
  obj_t *items;
  size_t count;
  size_t capacity;
} obj_list_t;

typedef struct {
  obj_t *items;
  size_t count;
} obj_root_t;

#define obj_string(s)                                                          \
  (obj_val_t) {                                                                \
    .type = OBJ_STRING, .string = {.ptr = (s), .len = sizeof(s) - 1}           \
  }

#define obj_int(n)                                                             \
  (obj_val_t) { .type = OBJ_INT, .integer = (n) }

#define obj_float(n)                                                           \
  (obj_val_t) { .type = OBJ_FLOAT, .floating = (n) }

#define obj_bool(b)                                                            \
  (obj_val_t) { .type = OBJ_BOOL, .boolean = (b) }

#define obj_array(...)                                                         \
  (obj_val_t) {                                                                \
    .type = OBJ_ARRAY, .array = {                                              \
      .items = (obj_val_t[]){__VA_ARGS__},                                     \
      .count = sizeof((obj_val_t[]){__VA_ARGS__}) / sizeof(obj_val_t)          \
    }                                                                          \
  }

#define obj_object(...)                                                        \
  (obj_val_t) {                                                                \
    .type = OBJ_OBJECT, .object = {                                            \
      .items = (obj_t[]){__VA_ARGS__},                                         \
      .count = sizeof((obj_t[]){__VA_ARGS__}) / sizeof(obj_t)                  \
    }                                                                          \
  }

#define obj_pair(k, v)                                                         \
  (obj_t) { .key = (k), .key_len = sizeof(k) - 1, .value = (v) }

#define obj_root(...)                                                          \
  &(obj_root_t){.items = (obj_t[]){__VA_ARGS__},                               \
                .count = sizeof((obj_t[]){__VA_ARGS__}) / sizeof(obj_t)}

#define obj_get(list, k) obj_get((list)->items, (list)->count, (k))

#define obj_put(list, k, v)                                                    \
  do {                                                                         \
    int found = 0;                                                             \
    for (size_t i = 0; i < (list)->count; i++) {                               \
      if (strcmp((list)->items[i].key, (k)) == 0) {                            \
        (list)->items[i].value = (v);                                          \
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
      (list)->items[(list)->count++] = obj_pair(k, v);                         \
    }                                                                          \
  } while (0)

#define obj_remove(list, k)                                                    \
  do {                                                                         \
    for (size_t i = 0; i < (list)->count; i++) {                               \
      if (strcmp((list)->items[i].key, (k)) == 0) {                            \
        (list)->items[i] = (list)->items[(list)->count - 1];                   \
        (list)->count--;                                                       \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
  } while (0)

static inline size_t serialize_object(char *buf, size_t size,
                                      const obj_t *items, size_t count);

static inline size_t serialize_val(char *buf, size_t size,
                                   const obj_val_t *val) {
  size_t offset = 0;
  switch (val->type) {
  case OBJ_STRING:
    offset += (size_t)snprintf(buf + offset, size - offset, "\"%.*s\"",
                               (int)val->string.len, val->string.ptr);
    break;
  case OBJ_INT:
    offset += (size_t)snprintf(buf + offset, size - offset, "%d", val->integer);
    break;
  case OBJ_FLOAT:
    offset +=
        (size_t)snprintf(buf + offset, size - offset, "%g", val->floating);
    break;
  case OBJ_BOOL:
    offset += (size_t)snprintf(buf + offset, size - offset, "%s",
                               val->boolean ? "true" : "false");
    break;
  case OBJ_ARRAY:
    offset += (size_t)snprintf(buf + offset, size - offset, "[");
    for (size_t i = 0; i < val->array.count; i++) {
      if (i > 0)
        offset += (size_t)snprintf(buf + offset, size - offset, ",");
      offset +=
          serialize_val(buf + offset, size - offset, &val->array.items[i]);
    }
    offset += (size_t)snprintf(buf + offset, size - offset, "]");
    break;
  case OBJ_OBJECT:
    offset += serialize_object(buf + offset, size - offset, val->object.items,
                               val->object.count);
    break;
  }
  return offset;
}

static inline size_t serialize_object(char *buf, size_t size,
                                      const obj_t *items, size_t count) {
  size_t offset = 0;
  offset += (size_t)snprintf(buf + offset, size - offset, "{");
  for (size_t i = 0; i < count; i++) {
    if (i > 0)
      offset += (size_t)snprintf(buf + offset, size - offset, ",");
    offset +=
        (size_t)snprintf(buf + offset, size - offset,
                         "\"%.*s\":", (int)items[i].key_len, items[i].key);
    offset += serialize_val(buf + offset, size - offset, &items[i].value);
  }
  offset += (size_t)snprintf(buf + offset, size - offset, "}");
  return offset;
}
