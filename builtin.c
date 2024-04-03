#include <stdio.h> // For input/output operations, e.g., printf() and perror()
#include <stdlib.h> // For general utilities like dynamic memory allocation, program control, environment information, etc. Might be used in more extensive parts of the shell or for future expansions.
#include <string.h> // For string operations. While not explicitly used in this snippet, often needed for command parsing and comparison.
#include <unistd.h> // For POSIX API, e.g., chdir() used in the cd command implementation.
#include "builtin.h"
#include <readline/readline.h>
#include <readline/history.h>

extern char *builtin_str[]; // Use the array from another file
extern int ksh_num_builtins(); // Use the function from another file

//cd implementation
int ksh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "ksh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("ksh");
    }
  }
  return 1;
}
//help implentation
int ksh_help(char **args)
{
  int i;
  printf("kinshell version 0.1\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < ksh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}
//exit implementation
int ksh_exit(char **args)
{
  return 0;
}
//history implementation
int ksh_history(char **args)
{
  HIST_ENTRY **the_history_list = history_list();

  if (the_history_list) {
  for (int i = 0; the_history_list[i]; i++) {
    printf("%d: %s\n", i + history_base, the_history_list[i]->line);
    }
  }
  return 1;
}