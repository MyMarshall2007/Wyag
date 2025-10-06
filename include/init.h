#ifndef INIT_H
#define INIT_H


#include "./main.h"
#include <stdbool.h>

char *join_path_d(char* parent, char* child);

void write_config(char *path, char *key, char **values, int len);

bool check_config(char *path);

char *create_repository(char *path);

char *parent_wd_d(char *path);

#endif