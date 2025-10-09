#include "../include/main.h"
#include "../include/init.h"
#include "../include/cat_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <dirent.h>

char *parent_wd_d(char *path) 
{
    struct stat st = {0};
    char *result = NULL;
    if (stat(path, &st) == -1) {
        perror("Invalid path directory.");
        return NULL;
    }

    size_t len_path = strlen(path) - 2;
    while (len_path >= 0) {
        if (path[len_path] == '/') 
            break;
        len_path--;
    }

    if (len_path < 2)
        return "/";
    
    result = (char *)malloc(len_path + 2);
    memcpy(result, path, len_path + 1);
    result[len_path+1] = '\0';

    return result;
}


char *repo_find_f(char *relative_cwd) 
{
    DIR *d;
    struct dirent *dir;
    struct stat st = {0};
    char *cwd = NULL;

    if (stat(relative_cwd, &st) == -1) {
        perror("It is not a valid directory.");
        return NULL;
    }

    if (strcmp(relative_cwd, "/") == 0) {
        perror("Didn't find the git repository.");
        return NULL;
    }
        
    d = opendir(relative_cwd);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".git") == 0) {
                closedir(d);
                cwd = join_path_d(relative_cwd, ".git/");
                free(relative_cwd);
                return cwd;
            }
        }
        closedir(d);
    }

    cwd = parent_wd_d(relative_cwd);
    free(relative_cwd);

    return repo_find_f(cwd);
}

char *object_join_path_d(char *path, char *sha)
{
    struct stat st = {0};
    if  (stat(path, &st) == -1) {
        perror("Not a valid path to a directory.");
        return NULL;
    }

    size_t len_head_sha = 3;
    char *head_sha = malloc(len_head_sha);
    if (!head_sha) {
        perror("Allocation error in objec_read_d.");
        return NULL;
    }
    memcpy(head_sha, sha, 2);
    head_sha[2] = '\0';

    char *part_one_path = join_path_d(path, head_sha);
    char *rest_path_obj = sha + len_head_sha - 1;
    char *full_path_obj = join_path_d(part_one_path, rest_path_obj);

    free(head_sha);
    free(part_one_path);

    return full_path_obj;
}

unsigned char *read_uncompressed_d(FILE *source) 
{
    z_stream strm;
    int ret;
    unsigned char in[COMPRESSED_CHUNK];
    unsigned char out[CHUNK];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    ret = inflateInit(&strm);
    if (ret != Z_OK) return NULL;

    strm.avail_in = fread(in, 1, COMPRESSED_CHUNK, source);
    if (ferror(source)) {
        (void)inflateEnd(&strm);
        return NULL;
    }
    if (strm.avail_in == 0) return NULL;
    strm.next_in = in;

    strm.avail_out = CHUNK;
    strm.next_out = out;
    ret = inflate(&strm, Z_NO_FLUSH);
    assert(ret != Z_STREAM_ERROR);
    switch (ret) {
        case Z_NEED_DICT:
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            (void)inflateEnd(&strm);
            return NULL;
    }

    (void)inflateEnd(&strm);
    unsigned char *buff = (unsigned char *)malloc(CHUNK);
    memcpy(buff, out, CHUNK);

    return buff;
}

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

