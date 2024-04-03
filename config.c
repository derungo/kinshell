#include "config.h"
#include <unistd.h> 
#include <pwd.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <readline/readline.h> 
#include <readline/history.h> 

// Function to get the path to the history file
char* get_history_file_path() {
    char* home_directory = getenv("HOME"); // Try to get the HOME environment variable
    if (!home_directory) {
        // Fallback to the passwd entry if HOME isn't set
        struct passwd* pw = getpwuid(getuid());
        home_directory = pw->pw_dir;
    }
    static char history_file_path[PATH_MAX];
    snprintf(history_file_path, PATH_MAX, "%s/.ksh_history", home_directory);
    return history_file_path;
}
// Function to load the history from file
void load_history_file() {
    char* history_file_path = get_history_file_path();
    if (read_history(history_file_path) < 0) {
        fprintf(stderr, "Failed to load history from %s\n", history_file_path);
    } 
}
// Function to save the history to file
void save_history_file() {
    char* history_file_path = get_history_file_path();
    if (write_history(history_file_path) < 0) {
        fprintf(stderr, "Failed to save history to %s\n", history_file_path);
    }
}