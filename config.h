#ifndef CONFIG_H
#define CONFIG_H

//added manual def of PATH_MAX
#ifndef PATH_MAX
#define PATH_MAX 4096 
#endif

// Function to get the path to the history file
char* get_history_file_path(void);
// Function to load the history from file
void load_history_file(void);
// Function to save the history to file
void save_history_file(void);
#endif