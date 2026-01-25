#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "../macros.h"
#include "../errors.h"
#include "../parse.h"
#define POSTGRES_IMPLEMENTATION
#include "postgres_driver/driver.h"

#define CUR_DIR                               "."
#define PARENT_DIR                            ".."
#define ALL_IN_DIR                            "*"
#define MIGRATION_FILE_NAME                   "mig.yaml"
#define YAML_APPLICATION_NAME                 "name"
#define YAML_APPLICATION_NAME_VALUE           "mig-cli-file"
#define YAML_APPLICATION_VERSION              "version"
#define YAML_APPLICATION_VERSION_VALUE        "1.0.0"
#define YAML_APPLICATION_MIGRATION            "migration"
#define YAML_APPLICATION_MIGRATION_PATH       "path"
#define YAML_APPLICATION_MIGRATION_PATH_VALUE "./"
#define YAML_APPLICATION_MIGRATION_NAME       "name"
#define YAML_APPLICATION_MIGRATION_NAME_VALUE "migration"
#define READ_WRITE_EXECUTE_PERMISSION         0755
#define STATIC_SIZE                           256
#define UP_SQL                                "up.sql"
#define DOWN_SQL                              "down.sql"
#define SQL_UP_MSG                            "-- Write the migration file for creation and modification here"
#define SQL_DOWN_MSG                          "-- Write the oppsite of what you wrote in the up.sql file"

typedef struct {
  char *small_value;
  char *big_value;
  void (*exec_cmd)(int argc, char **argv);
} Cli;

static char* get_migration_path(){
  char *path = malloc(STATIC_SIZE);
  
  char *f = parse_file(MIGRATION_FILE_NAME);
  YamlArray *conf = parse_yaml(f);
  const char *name = NULL, *dir = NULL;
  
  for (int i = 0; i < conf->count; ++i) {
      if (compare_string(conf->ymal[i].key, YAML_APPLICATION_MIGRATION_NAME))
          name = conf->ymal[i].value;
      else if (compare_string(conf->ymal[i].key, YAML_APPLICATION_MIGRATION_PATH))
          dir = conf->ymal[i].value;
  }

  int len = snprintf(path, STATIC_SIZE, "%s%s", dir, name);
  if(len > STATIC_SIZE)
    ERROR_EXIT(ALLOCATION_ERROR, "path");
  

  free(f);
  free_yaml(conf);
  return path;
}

static void exec_run(int argc, char **argv){
  char *path = get_migration_path(); 
  FOR_EACH_DIR_ENTRY(path, entry) {
      if (entry->d_type == DT_DIR) {
          char *subfolder_path = malloc(STATIC_SIZE);
          int len = sprintf(subfolder_path, "%s/%s", path, entry->d_name);
          if(len > STATIC_SIZE)
            ERROR_EXIT(ALLOCATION_ERROR, "subfolder_path"); 

          FOR_EACH_DIR_ENTRY(subfolder_path, file) {
              if (strstr(file->d_name, "up") != NULL) {
                  printf("Executing migration in folder: %s\n", entry->d_name);
                  printf("TODO: up\n");
              }
          }
      CLOSE_DIR(file)
      }
  } 
  CLOSE_DIR(entry)
  free(path);
}

static void exec_init(int argc, char **argv){
  FILE *f = fopen(MIGRATION_FILE_NAME, "w");
  fprintf(f, "%s: %s\n", YAML_APPLICATION_NAME, YAML_APPLICATION_NAME_VALUE);
  fprintf(f, "%s: %s\n", YAML_APPLICATION_VERSION, YAML_APPLICATION_VERSION_VALUE);
  fprintf(f, "%s:\n  %s: %s\n  %s: %s", 
          YAML_APPLICATION_MIGRATION, 
          YAML_APPLICATION_MIGRATION_PATH, 
          YAML_APPLICATION_MIGRATION_PATH_VALUE, 
          YAML_APPLICATION_MIGRATION_NAME, 
          YAML_APPLICATION_MIGRATION_NAME_VALUE
  );

  mkdir(YAML_APPLICATION_MIGRATION_NAME_VALUE, READ_WRITE_EXECUTE_PERMISSION);
  fclose(f);
  return;
}

static void exec_generate(int argc, char **argv){
  if(argc < 3 || argc>STATIC_SIZE)
    ERROR_EXIT("%s\n", INVALID_CMD);
  
  printf("GEN\n");
  char folder_name[STATIC_SIZE] = {0}, args_part[STATIC_SIZE] = {0}, time_part[STATIC_SIZE] = {0};
  char up_file[STATIC_SIZE] = {0}, down_file[STATIC_SIZE] = {0};
  char *path = get_migration_path();
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  for (int i = 2; i < argc; i++) {
    strcat(args_part, argv[i]);
    if (i < argc - 1)
      strcat(args_part, "_");
  }
  
  strftime(time_part, sizeof(time_part), "%Y-%m-%d-%H%M%S", t);
  snprintf(folder_name, sizeof(folder_name), "%s/%s_%s", path,time_part, args_part);
  
  mkdir(folder_name, READ_WRITE_EXECUTE_PERMISSION);
  snprintf(up_file, sizeof(up_file), "%s/%s", folder_name, UP_SQL);
  snprintf(down_file, sizeof(down_file), "%s/%s", folder_name, DOWN_SQL);

  FILE *f = fopen(up_file, "w");
  fprintf(f, "%s", SQL_UP_MSG);
  fclose(f);

  f = fopen(down_file, "w");
  fprintf(f, "%s", SQL_DOWN_MSG);
  fclose(f);

  free(path);
  printf("Migration succsessfully generated %s", folder_name);
}


static void exec_rollback(int argc, char **argv){
  printf("GEN\n");
}

static const Cli cli_commands[] ={
  {"r", "run", exec_run},
  {"i", "init", exec_init},
  {"g", "generate", exec_generate},
  {"rb", "rollback", exec_rollback}
};

int main(int argc, char **argv){
  int found;
  if (argc < 2) 
    ERROR_EXIT("%s", INVALID_CMD);

  
  FOREACH(const Cli *cmd, cli_commands){
    if(compare_string(argv[1], cmd->big_value) || compare_string(argv[1], cmd->small_value)){
      FOR_EACH_DIR_ENTRY(CUR_DIR, en) {
          if (cmd->exec_cmd != exec_init && !compare_string(en->d_name, MIGRATION_FILE_NAME))
              continue;
          found = 1;
          cmd->exec_cmd(argc, argv);
      }
      CLOSE_DIR(en);
    }
  }
  
  if(!found)
    ERROR_EXIT("%s", INVALID_CMD);
  
  return 0;
}
