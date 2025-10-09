#ifndef HASH_OBJECTS_AND_CAT_FILE
#define HASH_OBJECTS_AND_CAT_FILE

#include <stdio.h>
#include "./main.h"

/*  Take as input a current current directory and return the 
    directory within that path which contains the .git/ dir.
    Example:
        input: /home/marshallmallow/Folders/Personnal Project/Write yourself a git/Wyag/tests
        output: /home/marshallmallow/Folders/Personnal Project/Write yourself a git/Wyag/.git*/
char *parent_wd_d(char *path);

char *repo_find_f(char *relative_cwd);

char *object_join_path_d(char *path, char *sha);

unsigned char *read_uncompressed_d(FILE *source);

Object *object_read_d(char *repo, char *sha);

#endif