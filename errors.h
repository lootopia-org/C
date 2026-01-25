#pragma once
#include <stdlib.h>
#include <stdio.h>

#define CONFIG_ERROR              "Missing required config variable: %s\n"
#define ALLOCATION_ERROR          "Failed to allocate memory for variable %s\n"
#define POSTGRES_CONNECTION_ERROR "An error occured while trying to connect to the database %s\n"
#define FIND_FILE_ERROR           "Could not find file %s make sure it is in the current directory\n"
#define INVALID_CMD               "This command was invalid"

#define DIR_ERROR                 "The requested directory could not be found"
#define NAME_TOO_LONG             "The name for the genrated folder is to long"

#define PRINT_ERROR_DO_ACTION(msg, action, ...)     \
  do {                                              \
      fprintf(stderr, msg "\n", ##__VA_ARGS__);     \
      action;                                       \
  } while(0)

#define ERROR_EXIT(msg, ...) \
    PRINT_ERROR_DO_ACTION(msg, exit(EXIT_FAILURE), ##__VA_ARGS__)
#define ERROR(msg, ...) \
  PRINT_ERROR_DO_ACTION(msg, {}, ##__VA_ARGS__)

