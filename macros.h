#pragma once

#include <string.h>
#define GET_ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

#define CONCAT_AND_RET_STR(str1, str2) do {                       \
  const char *s1 = (str1);                                        \
  const char *s2 = (str2);                                        \
                                                                  \
  char *res = malloc(sizeof(char) + strlen(s1) + strlen(s2) + 1); \
  if(res){                                                        \
    memcpy(res, s1, strlen(s1));                                  \
    memcpy(res + strlen(s1), s2, strlen(s2) +1);                  \
  }                                                               \
                                                                  \
  res                                                             \
} while(0);                               
