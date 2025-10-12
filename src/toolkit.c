#include "../include/main.h"
#include <math.h>     
#include <time.h>        
#include <argp.h>       
#include <pwd.h>       
#include <grp.h>         
#include <fnmatch.h>    
#include <openssl/sha.h> 
#include <zlib.h>        
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <regex.h>       
#include <stdbool.h>
#include <assert.h>
#include <openssl/evp.h>

char *join_path_d(char* parent, char* child) 
{
    char *result;
    size_t len_parent = strlen(parent);
    size_t len_child = strlen(child);
    size_t total_len = len_child + len_parent + 1;
    bool need_separator = false;

    if (len_parent > 0 && parent[len_parent-1] != '/') {
        need_separator = true;
        total_len++;
    }
        
    result = (char *)malloc(total_len);
    if (!result) {
        perror("Allocation error.");
        return NULL;
    }

    char *current = result; 
    memcpy(current, parent, len_parent);
    current += len_parent;

    if (need_separator == true) {
        *current = '/';
        current++;
    }

    memcpy(current, child, len_child + 1);
    return result;


    // Don't ever do dynamic allocation in a condition, it will 
    // trigger a "Conditional jump or move depends on uninitialized values" 
    // and boy that shit nasty.
}


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
                return relative_cwd;
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
