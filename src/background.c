#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "shell.h"

extern BackgroundJob background_jobs[MAX_BACKGROUND_JOBS];
extern int job_counter;

// Add a background job to the tracking list
void add_background_job(pid_t pid, char *command_line) {
    for (int i = 0; i < MAX_BACKGROUND_JOBS; i++) {
        if (!background_jobs[i].active) {
            job_counter++;
            background_jobs[i].job_number = job_counter;
            background_jobs[i].pid = pid;
            strncpy(background_jobs[i].command_line, command_line, 
                    MAX_COMMAND_LENGTH - 1);
            background_jobs[i].command_line[MAX_COMMAND_LENGTH - 1] = '\0';
            background_jobs[i].active = true;
            return;
        }
    }
}

// Check for completed background jobs 
void check_background_jobs(void) {
    for (int i = 0; i < MAX_BACKGROUND_JOBS; i++) {
        if (background_jobs[i].active) {
            int status;
            pid_t result = waitpid(background_jobs[i].pid, &status, WNOHANG);
            
            if (result > 0) {
                // Process has finished check 
                printf("[%d]+ Done %s\n", 
                       background_jobs[i].job_number,
                       background_jobs[i].command_line);
                background_jobs[i].active = false;
            }
        }
    }
}

// Wait for all background jobs to complete - important as exit checks
void wait_for_background_jobs(void) {
    bool any_active = true;
    
    while (any_active) {
        any_active = false;
        
        for (int i = 0; i < MAX_BACKGROUND_JOBS; i++) {
            if (background_jobs[i].active) {
                any_active = true;
                int status;
                pid_t result = waitpid(background_jobs[i].pid, &status, WNOHANG);
                
                if (result > 0) {
                    printf("[%d]+ Done %s\n", 
                           background_jobs[i].job_number,
                           background_jobs[i].command_line);
                    background_jobs[i].active = false;
                } else {
                    sleep(100000);
                }
            }
        }
    }
}