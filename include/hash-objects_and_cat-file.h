#ifndef HASH_OBJECTS_AND_CAT_FILE
#define HASH_OBJECTS_AND_CAT_FILE

#include "./main.h"

char *repo_find_f(char *relative_cwd);

char *object_join_path_d(char *path, char *sha);

unsigned char *read_uncompressed_d(FILE *source);

Object *object_read_d(char *repo, char *sha);

#endif