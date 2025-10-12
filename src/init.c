#include "toolkit.c"
// #include "../include/init.h"
// #include <unistd.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <stdbool.h>
// #include <sys/stat.h>

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
    char *tmpdir = join_path_d(objectsdir, "tmp/");

    if (stat(gitdir, &st) == 0) {
        if (check_config(configfile) == true) 
            perror("Git directory already exist.");
        else 
            perror("Invalid config file. Cannot create new git repository.");
    } else {
        mkdir(gitdir, PERMISSION);
        mkdir(branchesdir, PERMISSION);
        mkdir(refsdir, PERMISSION);
        mkdir(objectsdir, PERMISSION);

        mkdir(headsdir, PERMISSION);
        mkdir(tagsdir, PERMISSION);
        mkdir(tmpdir, PERMISSION);

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
    free(tmpdir);

    return gitdir;
}