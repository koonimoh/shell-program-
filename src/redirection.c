#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "shell.h"

// Setup output
void setup_output_redirection(char* filename) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd == -1) {
        perror("open failed");
        exit(1); 
    }
    
    dup2(fd, STDOUT_FILENO); 
    close(fd);
}

// Setup input 
void setup_input_redirection(char* filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        exit(1);
    }
    
    dup2(fd, STDIN_FILENO);   
    close(fd);
}