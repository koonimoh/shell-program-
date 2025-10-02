#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <fnmatch.h>
#include <dirent.h>
#include <stdbool.h>
#include "shell.h"

// Expand environment variable from token starting with $
char* expand_var(char* token) {
    char* value = NULL;
    char* mod_token = NULL;
    char* found_path = NULL;
    
    if (token[0] == '$') {
        mod_token = token + 1;
        value = getenv(mod_token);
        if (value == NULL) {
            return strdup("");  
        }
        size_t val_len = strlen(value) + 1;
        char *directory = malloc(val_len);
        strcpy(directory, value);
        found_path = directory;
    } 
    return found_path;
}

// Process tilde expansion: ~ or ~/path
char* process_tilde_sign(char* token) {
    if (strcmp(token, "~") == 0) {
        char* home = getenv("HOME");
        return home ? strdup(home) : backup_home();
    }
    else if (token[1] == '/') {
        // "~/something" → "/home/username/something"
        char* home = getenv("HOME");
        if (home == NULL) {  
            home = backup_home();
            if (home == NULL) return strdup(token);
        }
        char* attaching_path = token + 2;  // Skip "~/"
        size_t combined_len = strlen(home) + strlen(attaching_path) + 2;
        char* processed_path = malloc(combined_len);
        snprintf(processed_path, combined_len, "%s/%s", home, attaching_path);
        return processed_path;
    }
    else {
        // "~username" → just return copy for now
        return strdup(token);
    }
}

// Backup method to get home directory using passwd
char* backup_home(void) {
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    
    if (pw != NULL) {
        return strdup(pw->pw_dir);
    } else {
        printf("backup_home_function failed\n");
        return NULL;
    }
}

bool contains_pattern(char *token) {
    return (strchr(token, '*') != NULL || 
            strchr(token, '?') != NULL ||
            strchr(token, '[') != NULL);
}

bool matches_pattern(char *pattern, char *filename) {
    return fnmatch(pattern, filename, 0) == 0;
}

char** expand_glob(char *pattern, int *count) {
    DIR *dir = opendir(".");
    struct dirent *entry;
    char **matches = malloc(400 * sizeof(char*));
    int directory_count = 0;

    if (dir == NULL) {
        *count = 0;
        return matches;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.' && pattern[0] != '.') {
            continue;
        }
        
        if (matches_pattern(pattern, entry->d_name)) {
            matches[directory_count++] = strdup(entry->d_name);
        }
    }

    closedir(dir);
    *count = directory_count;
    return matches;
}




void expand_tokens(tokenlist *tokens) {
    tokenlist *expanded = new_tokenlist();
    
    for (size_t i = 0; i < tokens->size; i++) {
        char *token = tokens->items[i];
        
        // Skip redirection operators and background operator
        if (strcmp(token, ">") == 0 || strcmp(token, "<") == 0 || 
            strcmp(token, "|") == 0 || strcmp(token, "&") == 0) {
            add_token(expanded, token);
            continue;
        }
        
        char *temp_token = NULL;
        
        if (token[0] == '$') {
            temp_token = expand_var(token);
            if (temp_token == NULL) {
                temp_token = strdup("");
            }
        }
        else if (token[0] == '~') {
            temp_token = process_tilde_sign(token);
        }
        else {
            temp_token = strdup(token);
        }
        
        if (contains_pattern(temp_token)) {
            int glob_count = 0;
            char** glob_results = expand_glob(temp_token, &glob_count);
            
            if (glob_count > 0) {
                for (int j = 0; j < glob_count; j++) {
                    add_token(expanded, glob_results[j]);
                    free(glob_results[j]);
                }
            } else {
                add_token(expanded, temp_token);
            }
            
            free(glob_results);
        } else {
            add_token(expanded, temp_token);
        }
        
        free(temp_token);
    }
    
    // Free old tokens items
    for (size_t i = 0; i < tokens->size; i++) {
        free(tokens->items[i]);
    }
    free(tokens->items);
    
    // Replace with expanded
    tokens->size = expanded->size;
    tokens->items = expanded->items;
    free(expanded);
}