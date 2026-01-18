#ifndef ERRORS_H
#define ERRORS_H

#include <stdlib.h>

#define PRINT_ERROR_DO_ACTION(msg, action) do { \
  fprintf(stderr, "%s\n", msg);                 \
  action;                                       \
} while(0)

#define ERROR_EXIT(msg) do {                      \
  PRINT_ERROR_DO_ACTION(msg, exit(EXIT_FAILURE)); \
} while(0)

#endif
