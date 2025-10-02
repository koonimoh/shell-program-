#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>
#include <sys/types.h>
#include "lexer.h"


#define MAX_COMMAND_LENGTH 200
#define MAX_BACKGROUND_JOBS 10
#define MAX_HISTORY 3
#define MAX_PIPES 100  


typedef struct {
    int job_number;
    pid_t pid;
    char command_line[MAX_COMMAND_LENGTH];
    bool active;
} BackgroundJob;


typedef struct {
    char **args;              
    int arg_count;
    char *input_file;       
    char *output_file;       
    bool background;        
    int pipe_count;          
    int *pipe_positions;     
} Command;


void display_prompt(void);


char* expand_var(char* token);
char* process_tilde_sign(char* token);
char* backup_home(void);


char* path_search(char* command);


void execute_external_command(Command *cmd);


void setup_input_redirection(char* filename);
void setup_output_redirection(char* filename);


void execute_with_pipes(Command *cmd);


void add_background_job(pid_t pid, char *command_line);
void check_background_jobs(void);
void wait_for_background_jobs(void);


bool is_builtin(char *command);
void execute_builtin(Command *cmd);
void builtin_exit(void);
void builtin_cd(char **args, int arg_count);
void builtin_jobs(void);


Command* parse_command(tokenlist *tokens);
void free_command(Command *cmd);
void expand_tokens(tokenlist *tokens);

void add_to_history(char *command);
void print_history(void);


bool contains_pattern(char *token);
bool matches_pattern(char *pattern, char *filename);
char** expand_glob(char *pattern, int *count);

#endif