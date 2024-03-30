#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <limits.h>
#include <pwd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096 // A common maximum path length on many systems
#endif

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

//function declars for builtins
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

//array of builtins
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

//cd implementation
int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}
//help implentation
int lsh_help(char **args)
{
  int i;
  printf("kinshell's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}
//exit implementation
int lsh_exit(char **args)
{
  return 0;
}

//command execution via fork and exec 
int lsh_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

//execute implentation
int lsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}

//readline implementation updated to use readline library for command history
char *lsh_read_line(void)
{
  char *line = readline("> ");
    if (line && *line) {
        add_history(line);
    }
    return line;
}


//tokenization of input
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

//command input loop
void lsh_loop(void) {
    char *line;
    char **args;
    int status;
    char cwd[PATH_MAX]; // Using PATH_MAX for portability
    char *user;
    char prompt[PATH_MAX + 50]; // Ensure buffer is large enough for prompt

    do {
        // Get current working directory
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("lsh");
            exit(EXIT_FAILURE);
        }

        // Get username
        user = getenv("USER");
        if (user == NULL) {
            struct passwd *pw = getpwuid(getuid());
            if (pw == NULL) {
                perror("lsh: unable to fetch username");
                exit(EXIT_FAILURE);
            }
            user = pw->pw_name;
        }

        // Construct and display prompt
        snprintf(prompt, sizeof(prompt), "[%s@%s]$ ", user, cwd);

        // Read line from user input using readline
        line = readline(prompt);
        if (line && *line) {
            add_history(line); // Add the line to history only if it's not empty
        }

        // Split line into args
        args = lsh_split_line(line);
        
        // Execute command
        status = lsh_execute(args);

        // Free memory
        free(line);
        free(args);
    } while (status);
}

//main loop
int main(int argc, char **argv)
{
//load config files
//run command loop
lsh_loop();
//shutdown/cleanup
return EXIT_SUCCESS;
}
