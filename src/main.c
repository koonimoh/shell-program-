#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shell.h"
#include "lexer.h"

// Global variables for shell state
BackgroundJob background_jobs[MAX_BACKGROUND_JOBS];
int job_counter = 0;
char *command_history[MAX_HISTORY];
int history_count = 0;

int main(void) {
    // Initialize background jobs
    for (int i = 0; i < MAX_BACKGROUND_JOBS; i++) {
        background_jobs[i].active = false;
    }
    
    // Initialize history
    for (int i = 0; i < MAX_HISTORY; i++) {
        command_history[i] = NULL;
    }
    
    while (1) {
        // Check for completed background jobs
        check_background_jobs();
        
        // Display prompt
        display_prompt();
        
        // Get input using lexer
        char *input = get_input();
        
        // Skip empty input
        if (input == NULL || strlen(input) == 0) {
            free(input);
            continue;
        }
        
        // Tokenize using lexer
        tokenlist *tokens = get_tokens(input);
        
        //  if no tokens
        if (tokens->size == 0) {
            free(input);
            free_tokens(tokens);
            continue;
        }
        
        // Expand environment variables and tildes
        expand_tokens(tokens);
        
        // Parse command structure
        Command *cmd = parse_command(tokens);
        
        // Check for built-in commands 
        if (is_builtin(cmd->args[0])) {
            // Add to history before executing
            add_to_history(input);
            
            execute_builtin(cmd);
            
            //  for exit
            if (strcmp(cmd->args[0], "exit") == 0) {
                free(input);
                free_tokens(tokens);
                free_command(cmd);
                break;
            }
        } else {
            // Add to history 
            add_to_history(input);
            
            // Execute external command
            if (cmd->pipe_count > 0) {
                execute_with_pipes(cmd);
            } else {
                execute_external_command(cmd);
            }
        }
        
        // Cleanup
        free(input);
        free_tokens(tokens);
        free_command(cmd);
    }
    
    // Final cleanup
    for (int i = 0; i < MAX_HISTORY; i++) {
        if (command_history[i]) free(command_history[i]);
    }
    
    return 0;
}