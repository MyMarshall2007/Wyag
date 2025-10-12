#include "toolkit.c"
// #include <stdio.h>
// #include <stdlib.h>
// #include <zlib.h>
// #include <assert.h>
// #include <string.h>
// #include <stdbool.h>
// #include <sys/stat.h>
// #include <dirent.h>

Object *object_read_d(char *obj_dir, char *sha)
{
    struct stat st = {0};
    char *path = NULL;
    if (stat(obj_dir, &st) == -1) {
        perror("Not a valid path to a directory.");
        return NULL;
    }

    path = object_join_path_d(obj_dir, sha);
    FILE *git_obj = fopen(path, "rb");
    free(path);
    if (!git_obj) {
        perror("Error when opening file.");
        return NULL;
    }

    unsigned char *buffer = read_uncompressed_d(git_obj);
    fclose(git_obj);
    if (buffer == NULL) {
        perror("Error when decompressing file.");
        return NULL;
    }

    int end = 0;
    for (; end < CHUNK; end++) {
        if (buffer[end] == '\0')
            break;
    }
    if (end == CHUNK -1) {
        perror("Couldn't find the header of object.");
        free(buffer);
        return NULL;
    }

    char header[end+1];
    memcpy(header, (char *)buffer, end);
    free(buffer);
    header[end] = '\0';

    char *obj_frmt = strtok(header, " ");
    char *size_str = header + strlen(obj_frmt) + 1;

    enum OBJECT_FORMAT format = NO_FORMAT;
    char *frmt_arr[] = {"commit", "tree", "blob"};
    int i;
    for (i = 0; i < 3; i++) {
        if (strcmp(frmt_arr[i], obj_frmt) == 0) {
            format = i;
            break;
        }
    }

    size_t size = (size_t)atoi(size_str);
    if (size == 0) {
        perror("Empyt file.");
        return NULL;
    }

    Object *obj = malloc(sizeof(Object));
    if (!obj) {
        perror("Object allocation error.");
        return NULL;
    }

    memcpy(obj->sha, sha, SHA_LENGTH);
    obj->format = format;
    obj->size = size;
    
    return obj;
}

