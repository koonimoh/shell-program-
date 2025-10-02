#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "shell.h"

// External declaration of background jobs from main.c
extern BackgroundJob background_jobs[MAX_BACKGROUND_JOBS];
extern int job_counter;


void execute_external_command(Command *cmd) {
    if (cmd->args[0] == NULL) {
        return;
    }
    
    // Search for executable path
    char *exec_path = path_search(cmd->args[0]);
    
    if (exec_path == NULL) {
        fprintf(stderr, "-bash: %s: command not found\n", cmd->args[0]);
        return;
    }
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        
        
        if (cmd->input_file != NULL) {
            setup_input_redirection(cmd->input_file);
        }
        
        // Setup output redirection if specified
        if (cmd->output_file != NULL) {
            setup_output_redirection(cmd->output_file);
        }
        
        
        execv(exec_path, cmd->args);
        
        // If execv returns, an error occurred
        perror("execv");
        exit(1);
        
    } else if (pid > 0) {
        // Parent process
        
        if (cmd->background) {
            
            char command_line[MAX_COMMAND_LENGTH] = "";
            for (int i = 0; cmd->args[i] != NULL; i++) {
                strcat(command_line, cmd->args[i]);
                if (cmd->args[i + 1] != NULL) {
                    strcat(command_line, " ");
                }
            }
            if (cmd->input_file) {
                strcat(command_line, " < ");
                strcat(command_line, cmd->input_file);
            }
            if (cmd->output_file) {
                strcat(command_line, " > ");
                strcat(command_line, cmd->output_file);
            }
            
            add_background_job(pid, command_line);
            printf("[%d] %d\n", job_counter, pid);
        } else {
            //wait for child
            int status;
            waitpid(pid, &status, 0);
        }
        
    } else {
        perror("fork");
    }
    
    free(exec_path);
}