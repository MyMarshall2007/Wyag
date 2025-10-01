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

#define PERMISSION 0755
#define INIT "init"

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

void write_config(char *path, char *key, char **values, int len)
{
    FILE *config_file = fopen(path, "w");
    fprintf(config_file, "[%s]\n", key);
    
    for (int i = 0; i < len; i++) {
        fprintf(config_file, "%s\n", values[i]);
    }

    fclose(config_file);
}

bool check_config(char *path)
{
    FILE *config_file = fopen(path, "r");
    char *line = NULL;
    size_t buff_size = 0;
    size_t read;

    bool stop = false;
    while (stop != true)
    {
        read = getline(&line, &buff_size, config_file);
        if (strcmp(line, "repositoryformatversion = 0\n") == 0) {
            stop = true;
        } 

        if (read == -1) {
            break;
        }

        free(line);
        line = NULL;
    }
    fclose(config_file);
    return stop;
}

char *create_repository(char *path)
{
    struct stat st = {0};
    char *gitdir = NULL;
    char *cwd = NULL;

    if (strcmp(path, ".") == 0) {
        cwd = getcwd(NULL, 0);
        gitdir = join_path_d(cwd, ".git/");
        free(cwd);
    } else {
        if (stat(path, &st) == -1) {
            perror("The directory does not exist.");
            return NULL;
        }    
        else {
            gitdir = join_path_d(path, ".git/");
            printf("%s\n", gitdir);
        }
    }

    char *objectsdir = join_path_d(gitdir, "objects/");
    char *branchesdir = join_path_d(gitdir, "branches/");
    char *refsdir = join_path_d(gitdir, "refs/");
    
    char *configfile = join_path_d(gitdir, "config");
    char *HEADfile = join_path_d(gitdir, "HEAD");
    char *descriptionfile = join_path_d(gitdir, "description");

    char *headsdir = join_path_d(refsdir, "heads/");
    char *tagsdir = join_path_d(refsdir, "tags/");

    if (stat(gitdir, &st) == 0) {
        if (check_config(configfile) == true) 
            perror("Git directory already exist.");
        else 
            perror("Invalid config file. Cannot create new git repository.");
    } else {
        mkdir(gitdir, PERMISSION);
        mkdir(branchesdir, PERMISSION);
        mkdir(refsdir, PERMISSION);

        mkdir(headsdir, PERMISSION);
        mkdir(tagsdir, PERMISSION);

        char *key = "core";
        char *values[3] = {
            "repositoryformatversion = 0",
            "filemode = false",
            "bare = false"
        };
        write_config(configfile, key, values, 3);

        FILE *description = fopen(descriptionfile, "w");
        fprintf(description, "Unnamed repository; edit this file 'description' to name the repository.\n");
        fclose(description);

        FILE *HEAD = fopen(HEADfile, "w");
        fprintf(HEAD, "ref: refs/heads/master\n");
        fclose(HEAD);
    }

    free(objectsdir);
    free(branchesdir);
    free(refsdir);
    free(headsdir);
    free(tagsdir);
    free(configfile);
    free(descriptionfile);
    free(HEADfile);

    return gitdir;
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


char *repo_find(char *relative_cwd) 
{
    DIR *d;
    struct dirent *dir;
    char *cwd = NULL;

    if (strcmp(relative_cwd, ".") == 0) {
        if ((cwd = getcwd(NULL, 0)) != NULL)
            relative_cwd = cwd;
        else
            perror("Error when trying to get the current working directory.");
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
                return join_path_d(relative_cwd, ".git/");
            }
        }
        closedir(d);
    }

    cwd = parent_wd_d(relative_cwd);
    free(relative_cwd);

    return repo_find(cwd);
}

int main(int argc, char *argv[])
{
    char *path = create_repository(".");
    if (path != NULL)
        printf("%s\n", path);

    free(path);

    char *result = repo_find("./test1/test2/test3");
    printf("The root git repository is: %s\n", result);
    free(result);

    return 0;
}

