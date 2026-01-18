#pragma once
#include <stdlib.h>

#define CONFIG_ERROR              "Missing required config variable: %s\n"
#define ALLOCATION_ERROR          "Failed to allocate memory for variable %s\n"
#define POSTGRES_CONNECTION_ERROR "An error occured while trying to connect to the database %s\n"

#define PRINT_ERROR_DO_ACTION(msg, action, ...)     \
  do {                                              \
      fprintf(stderr, msg "\n", ##__VA_ARGS__);     \
      action;                                       \
  } while(0)

#define ERROR_EXIT(msg, ...) \
    PRINT_ERROR_DO_ACTION(msg, exit(EXIT_FAILURE), ##__VA_ARGS__)

