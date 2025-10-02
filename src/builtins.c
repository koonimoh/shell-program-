#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "shell.h"

extern BackgroundJob background_jobs[MAX_BACKGROUND_JOBS];

// Check if command is a built-in
bool is_builtin(char *command) {
    return (strcmp(command, "exit") == 0 ||
            strcmp(command, "cd") == 0 ||
            strcmp(command, "jobs") == 0);
}

// Execute built-in command
void execute_builtin(Command *cmd) {
    if (strcmp(cmd->args[0], "exit") == 0) {
        builtin_exit();
    } else if (strcmp(cmd->args[0], "cd") == 0) {
        builtin_cd(cmd->args, cmd->arg_count);
    } else if (strcmp(cmd->args[0], "jobs") == 0) {
        builtin_jobs();
    }
}

// Built-in: exit 
void builtin_exit(void) {
    printf("Waiting for background jobs to complete...\n");
    wait_for_background_jobs();
    print_history();
}

// Built-in: cd 
void builtin_cd(char **args, int arg_count) {
    char *target = NULL;
    
    //  go to HOME
    if (arg_count == 1) {
        target = getenv("HOME");
        if (target == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return;
        }
    }
    // More than one arg: error
    else if (arg_count > 2) {
        fprintf(stderr, "cd: too many arguments\n");
        return;
    }
    // One argument: use it as target
    else {
        target = args[1];
    }
    
    // Check if target exists
    struct stat st;
    if (stat(target, &st) != 0) {
        fprintf(stderr, "cd: %s: No such file or directory\n", target);
        return;
    }
    
    // Check if target is a directory
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "cd: %s: Not a directory\n", target);
        return;
    }
    
    // Change directory
    if (chdir(target) != 0) {
        perror("cd");
    }
}

// Built-in: jobs 
void builtin_jobs(void) {
    bool found_any = false;
    
    for (int i = 0; i < MAX_BACKGROUND_JOBS; i++) {
        if (background_jobs[i].active) {
            printf("[%d]+ %d %s\n", 
                   background_jobs[i].job_number,
                   background_jobs[i].pid,
                   background_jobs[i].command_line);
            found_any = true;
        }
    }
    
    if (!found_any) {
        printf("No active background jobs\n");
    }
}