#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>

// Display the shell prompt: USER@MACHINE:PWD>
void display_prompt(void) {
    // Get user with passwd fallback
    const char *user = getenv("USER");
    if (!user) {
        struct passwd *pw = getpwuid(geteuid());
        user = pw ? pw->pw_name : "unknown";
    }
    
    // Get hostname - try env first
    char machine[HOST_NAME_MAX + 1];
    char *env_host = getenv("HOSTNAME");
    if (env_host == NULL) {
        env_host = getenv("HOST");
    }
    
    if (env_host != NULL) {
        snprintf(machine, sizeof(machine), "%s", env_host);
    } else {
        if (gethostname(machine, sizeof(machine)) != 0) {
            snprintf(machine, sizeof(machine), "unknown");
        }
    }
    
    // Get cwd for pwd
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        strcpy(cwd, "unknown");
    }
    
    // Print prompt format: USER@MACHINE:PWD>
    printf("%s@%s:%s> ", user, machine, cwd);
    fflush(stdout);
}