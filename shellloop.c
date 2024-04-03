#include <unistd.h> // For getcwd(), gethostname(), and possibly other POSIX-like system calls
#include <stdlib.h> // For malloc(), free(), exit(), and getenv()
#include <stdio.h> // For perror() and snprintf()
#include <string.h> // For strlen(), strncmp(), and strdup()
#include <limits.h> // For PATH_MAX
#include <pwd.h> // For getpwuid() and struct passwd
#include <readline/readline.h> // For readline()
#include <readline/history.h> // For add_history()

//added manual def of PATH_MAX
#ifndef PATH_MAX
#define PATH_MAX 4096 
#endif

#include "shellloop.h"
#include "parse.h"
#include "execute.h"

//command input loop
void ksh_loop(void) {
    char *line;
    char **args;
    int status;
    char cwd[PATH_MAX]; // Using PATH_MAX for portability
    char *user;
    char prompt[PATH_MAX + 50]; // Ensure buffer is large enough for prompt
    char hostname[1024]; // hostname buffer
    char *home_directory;
    char *relative_path;

    hostname[1023] = '\0'; // Ensure null termination
    gethostname(hostname, 1023); // Get the hostname

    do {
        // Get current working directory
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("ksh");
            exit(EXIT_FAILURE);
        }

        // Get username
        user = getenv("USER");
        if (user == NULL) {
            struct passwd *pw = getpwuid(getuid());
            if (pw == NULL) {
                perror("ksh: unable to fetch username");
                exit(EXIT_FAILURE);
            }
            user = pw->pw_name;
        }
        // attempt to abbreviate home directory
        home_directory = getenv("HOME");
        if (home_directory && strncmp(cwd, home_directory, strlen(home_directory)) == 0) {
            // If cwd contains the home directory, replace it with '~' for abbreviation
            int len = strlen(cwd) - strlen(home_directory) + 2; 
            relative_path = malloc(len);
            if (relative_path == NULL) {
                perror("ksh: unable to allocate memory");
                exit(EXIT_FAILURE);
            }
            //relative path starts with ~
            snprintf(relative_path, len, "~%s", cwd + strlen(home_directory));
        } else {
            // Allocate and copy the cwd as is if it doesn't start with home_director
            relative_path = strdup(cwd);
            if(relative_path == NULL) {
              perror("ksh: unable to allocate memory");
              exit(EXIT_FAILURE);
            }
        }


        // Construct and display prompt
        snprintf(prompt, sizeof(prompt), "[%s@%s]-[%s] $ ", user, hostname, relative_path);

        // Read line from user input using readline
        line = readline(prompt);
        if (line && *line) {
            add_history(line); // Add the line to history only if it's not empty
        }

        // Split line into args
        args = ksh_split_line(line);
        
        // Execute command
        status = ksh_execute(args);

        // Free memory
        free(line);
        free(args);
        free(relative_path);
    } while (status);
}