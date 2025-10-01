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

#define SEPARATOR '/'
#define PERMISSION 0755
#define INIT "init"

char *join_path_d(char* parent, char* child) 
{
    char *result;
    size_t len_parent = strlen(parent);
    size_t len_child = strlen(child);
    size_t len_buff = len_parent + len_child; 

    result = malloc(len_buff); 
    strcpy(result, parent);
    result[len_parent] = SEPARATOR;
    strcpy(result + len_parent + 1, child);

    return result;
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

    if (strcmp(path, ".") == 0) {
        char cwd[_PC_PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        gitdir = join_path_d(cwd, ".git/");
    }

    if (stat(path, &st) == -1)  {
        perror("The directory does not exist.");
    } else {
        gitdir = join_path_d(path, "./git");
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



char *repo_find(char *relative_cwd) 
{
    struct stat st = {0};
    char *cwd = NULL;

    if (strcmp(relative_cwd, ".") == 0) {
        cwd = malloc(_PC_PATH_MAX);
        getcwd(cwd, sizeof(cwd));
    } else 
        cwd = relative_cwd;

    if (stat("./.git/", &st) == 0) {
        return join_path_d(cwd, ".git/");
    }

    char *parent = join_path_d(cwd, "../");
    if (strcmp(cwd, parent) == 0) {
        perror("No root git repository.");
    }

    return repo_find(parent);
}

int main(int argc, char *argv[])
{
    char *path = NULL;
    if (argc == 1)
        perror("Enter a git argument.");
    
    else if (argc == 2) {
        if (strcmp(INIT, argv[1]) == 0) {
            path = create_repository(".");
            printf("Git repository created in the current working directory: %s\n", path);
            free(path);
        } 
        else 
            perror("Invalid argument.\n");
    }

    else if (argc == 3) {
        if (strcmp(INIT, argv[1]) == 0) {
            char *path = create_repository(argv[2]);
            printf("Git repository created at: %s\n", argv[2]);
        } 
        else 
            perror("Invalid git argument.\n");
    }

    else 
        perror("Too many argument.\n");

    return 0;
}

