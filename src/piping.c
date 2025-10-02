#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "shell.h"

extern BackgroundJob background_jobs[MAX_BACKGROUND_JOBS];
extern int job_counter;

// Execute command with pipes 
void execute_with_pipes(Command *cmd) {
    int num_commands = cmd->pipe_count + 1;
    int pipes[MAX_PIPES][2];
    pid_t pids[MAX_PIPES];
    
    // Create all pipes
    for (int i = 0; i < cmd->pipe_count; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return;
        }
    }
    
    // Execute each command in the pipeline
    int arg_start = 0;
    for (int i = 0; i < num_commands; i++) {
       
        int arg_end = (i < cmd->pipe_count) ? cmd->pipe_positions[i] : cmd->arg_count;
        
        // Build arg array for this command
        int cmd_argc = arg_end - arg_start;
        char **cmd_argv = malloc((cmd_argc + 1) * sizeof(char*));
        for (int j = 0; j < cmd_argc; j++) {
            cmd_argv[j] = cmd->args[arg_start + j];
        }
        cmd_argv[cmd_argc] = NULL;
        
        // Search for exe
        char *exec_path = path_search(cmd_argv[0]);
        if (exec_path == NULL) {
            fprintf(stderr, "-bash: %s: command not found\n", cmd_argv[0]);
            free(cmd_argv);
            
            // Close all pipes
            for (int j = 0; j < cmd->pipe_count; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            return;
        }
        
        pids[i] = fork();
        
        if (pids[i] == 0) {
            // Child process
            
           
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            } else if (cmd->input_file != NULL) {
                
                setup_input_redirection(cmd->input_file);
            }
            
            // Setup output to next pipe
            if (i < cmd->pipe_count) {
                dup2(pipes[i][1], STDOUT_FILENO);
            } else if (cmd->output_file != NULL) {
                // Last command can have output redirection
                setup_output_redirection(cmd->output_file);
            }
            
            // Close all pipe file descriptors
            for (int j = 0; j < cmd->pipe_count; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            execv(exec_path, cmd_argv);
            perror("execv");
            exit(1);
        } else if (pids[i] < 0) {
            perror("fork");
            free(exec_path);
            free(cmd_argv);
            return;
        }
        
        free(exec_path);
        free(cmd_argv);
        arg_start = arg_end + 1; 
    }
    
    // Parent: close all pipes
    for (int i = 0; i < cmd->pipe_count; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    
    if (cmd->background) {
        // Background: add last process to jobs
        char command_line[MAX_COMMAND_LENGTH] = "";
        for (int i = 0; cmd->args[i] != NULL; i++) {
            if (strcmp(cmd->args[i], "|") != 0) {
                strcat(command_line, cmd->args[i]);
                strcat(command_line, " ");
            } else {
                strcat(command_line, "| ");
            }
        }
        
        add_background_job(pids[num_commands - 1], command_line);
        printf("[%d] %d\n", job_counter, pids[num_commands - 1]);
    } else {
        // Foreground: wait for all processes
        for (int i = 0; i < num_commands; i++) {
            int status;
            waitpid(pids[i], &status, 0);
        }
    }
}