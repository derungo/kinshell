#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <pwd.h>

//added manual def of PATH_MAX
#ifndef PATH_MAX
#define PATH_MAX 4096 
#endif

//include headers
#include "shellloop.h"
#include "builtin.h"
#include "parse.h"
#include "execute.h"
#include "completion.h"
#include "config.h"

//array of builtins
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "history",
  NULL
};

//builtin func pointers
int (*builtin_func[]) (char **) = {
  &ksh_cd,
  &ksh_help,
  &ksh_exit,
  &ksh_history
};
//returns number of builtins
int ksh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

//readline implementation using readline library for command history
char *ksh_read_line(void)
{
  char *line = readline("> ");
    if (line && *line) {
        add_history(line);
    }
    return line;
}

//main loop
int main(int argc, char **argv)
{
//call readline
initialize_readline();
//load history
load_history_file();
//call command loop
ksh_loop();
//call cleanup
save_history_file(); 
//shutdown/cleanup
return EXIT_SUCCESS;
}
