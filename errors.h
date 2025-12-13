#ifndef ERRORS_H
#define ERRORS_H

#include <stdlib.h>

#define ERROR_EXIT(msg) do { \
  fprintf(stderr, "%s\n", msg); \
  exit(EXIT_FAILURE); \
  } while(0)

#endif
