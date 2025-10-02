#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"

// Search for executable in $PATH directories
char* path_search(char* command) {
    // If command contains '/', don't search PATH
    if (strchr(command, '/') != NULL) {
        if (access(command, X_OK) == 0) {
            return strdup(command);
        }
        return NULL;
    }
    
    char* path_env = getenv("PATH");
    
    if (path_env == NULL) {
        return NULL;  
    }
    
    char* path = strdup(path_env);
    if (path == NULL) {
        return NULL;  
    }
    
    char* split_directory = strtok(path, ":");
    while (split_directory != NULL) {
        size_t fullpath_len = strlen(command) + 1 + strlen(split_directory) + 1;
        char* full_path = malloc(fullpath_len);
        if (full_path == NULL) {
            // malloc check 
            perror("Malloc failed, check path_search function");
            free(path);
            return NULL;
        }
        snprintf(full_path, fullpath_len, "%s/%s", split_directory, command);
        if (access(full_path, X_OK) == 0) {
            free(path);
            return full_path;
        }
        free(full_path);
        split_directory = strtok(NULL, ":");
    }
    free(path);
    return NULL;
}