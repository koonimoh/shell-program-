#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"

extern char *command_history[MAX_HISTORY];
extern int history_count;

// Parse tokenlist into Command structure
Command* parse_command(tokenlist *tokens) {
    Command *cmd = malloc(sizeof(Command));
    cmd->args = malloc((tokens->size + 1) * sizeof(char*));
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->background = false;
    cmd->pipe_count = 0;
    cmd->pipe_positions = malloc(MAX_PIPES * sizeof(int));
    
    int arg_index = 0;
    
    for (int i = 0; i < tokens->size; i++) {
        char *token = tokens->items[i];
        
        // Check for input
        if (strcmp(token, "<") == 0) {
            if (i + 1 < tokens->size) {
                cmd->input_file = strdup(tokens->items[i + 1]);
                i++;  
            }
        }
        // Check for output 
        else if (strcmp(token, ">") == 0) {
            if (i + 1 < tokens->size) {
                cmd->output_file = strdup(tokens->items[i + 1]);
                i++;  // Skip filename
            }
        }
        // Check for pipe
        else if (strcmp(token, "|") == 0) {
            cmd->pipe_positions[cmd->pipe_count] = arg_index;
            cmd->pipe_count++;
            cmd->args[arg_index++] = strdup(token);  // Keep pipe in args for reconstruction
        }
        // Check for background
        else if (strcmp(token, "&") == 0) {
            cmd->background = true;
        }
        // Regular arg
        else {
            cmd->args[arg_index++] = strdup(token);
        }
    }
    
    cmd->args[arg_index] = NULL;
    cmd->arg_count = arg_index;
    
    return cmd;
}

// Free Command structure
void free_command(Command *cmd) {
    if (cmd == NULL) return;
    
    for (int i = 0; i < cmd->arg_count; i++) {
        if (cmd->args[i]) free(cmd->args[i]);
    }
    free(cmd->args);
    
    if (cmd->input_file) free(cmd->input_file);
    if (cmd->output_file) free(cmd->output_file);
    if (cmd->pipe_positions) free(cmd->pipe_positions);
    
    free(cmd);
}

// Add command to history (keeps last 3)
void add_to_history(char *command) {
    // Don't add empty commands
    if (command == NULL || strlen(command) == 0) {
        return;
    }
    
    // shift for recency
    if (history_count >= MAX_HISTORY) {
        free(command_history[0]);
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            command_history[i] = command_history[i + 1];
        }
        command_history[MAX_HISTORY - 1] = strdup(command);
    } else {
        command_history[history_count] = strdup(command);
        history_count++;
    }
}

// Print command history (for exit command)
void print_history(void) {
    if (history_count == 0) {
        printf("No commands in history\n");
        return;
    }
    
    printf("Last %d command(s):\n", history_count > 3 ? 3 : history_count);
    
    int start = (history_count > 3) ? history_count - 3 : 0;
    for (int i = start; i < history_count; i++) {
        printf("%d: %s\n", i - start + 1, command_history[i]);
    }
}