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

int main() 
{
    char *path = "/home/marshallmallow/Folders/Personnal Project/Write yourself a git/test1/test2/test3";
    char *result = parent_wd_d(path);
    printf("%s\n", result);
    
    free(result);
    return 0;
}