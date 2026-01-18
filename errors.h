#ifndef ERRORS_H
#define ERRORS_H

#include <stdlib.h>

#define POSTGRES_CONNECTION_ERROR "An error occured while trying to connect to the database %s"

#define PRINT_ERROR_DO_ACTION(msg, action, ...)     \
  do {                                              \
      fprintf(stderr, msg "\n", ##__VA_ARGS__);     \
      action;                                       \
  } while(0)

#define ERROR_EXIT(msg, ...) \
    PRINT_ERROR_DO_ACTION(msg, exit(EXIT_FAILURE), ##__VA_ARGS__)

#endif
