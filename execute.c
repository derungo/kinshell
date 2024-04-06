#include "execute.h"
#include "builtin.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>


extern char *builtin_str[]; // Use the array from another file
extern int ksh_num_builtins(); // Use the function from another file
extern int (*builtin_func[]) (char **); //extern int (*builtin_func[]) (char **);

//command execution via fork and exec 
int ksh_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("ksh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("ksh");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
      if (wpid == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

//execute implentation
int ksh_execute(char **args)
{
  //int i;

  if (args[0] == NULL || builtin_str[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

   for (int i = 0; builtin_str[i] != NULL; i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

  return ksh_launch(args);
}
