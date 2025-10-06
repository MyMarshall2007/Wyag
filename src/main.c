#include "../include/main.h"
#include "../include/init.h"

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
