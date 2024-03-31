#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <limits.h> //this was supposed to be for PATH_MAX but it wasn't working for some reason
#include <pwd.h>
#include <glob.h>

//added manual def of PATH_MAX
#ifndef PATH_MAX
#define PATH_MAX 4096 
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
  "exit",
  NULL
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
//utility func for determing context of input
int determine_completion_context(const char* text, int start){
  //if start index is 0, likely command
  if(start == 0){
    return 1;
  }
  //if last char before current pos is space, likely arg
  if (start > 1 && text[start - 1] == ' ') {
    return 2;
  }
  //default filename
  return 3;
  }
//filename tab completion
char* filename_completion_generator(const char* text, int state) {
  static glob_t glob_result;
  static int list_index;

  if(!state){
    //prepare pattern for glob
    char pattern[PATH_MAX];
    snprintf(pattern, sizeof(pattern), "%s", text);
    glob(pattern, GLOB_TILDE, NULL, &glob_result);
    list_index = 0;
  }

  if(list_index >= glob_result.gl_pathc){
    globfree(&glob_result);
    return NULL;
  } else {
    return strdup(glob_result.gl_pathv[list_index++]);
      }

}
//tab builtin command completeion function
char* completion_generator(const char* text, int state) {
  
  //static vars to hold state across function calls
  static int list_index, len;
  char *name;
  //if new word, init state
  if (!state) {
    list_index = 0;
    len = strlen(text);
  }
  //return next name in list
  while ((name = builtin_str[list_index++]) !=NULL) {
    if (strncmp(name, text, len) == 0) {
      return name;
    }
  }
  return NULL;
}
//arg completion function
char* arg_completion_generator(const char* text, int state) {
 static int list_index, len;
 const char *args[] = {"on", "off", NULL};
 const char *name;
 
 if(!state){
   list_index = 0;
   len = strlen(text);
 }
 
 while((name = args[list_index++]) != NULL){
   if(strncmp(name, text, len) == 0){
     return strdup(name);
   }
 }
   return NULL;
}
//completion entrypoint
char** shell_completion(const char* text, int start, int end) {
    rl_attempted_completion_over = 1;
    int context = determine_completion_context(rl_line_buffer, rl_point);

    switch(context){
    case 1:
      return rl_completion_matches(text, completion_generator);
    case 2:
      return rl_completion_matches(text, arg_completion_generator);
    default:
      return rl_completion_matches(text, filename_completion_generator);
}
}
void initialize_readline(void) {
  rl_readline_name = "Kinshell";
  rl_attempted_completion_function = shell_completion;
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

  if (args[0] == NULL || builtin_str == NULL) {
    // An empty command was entered.
    return 1;
  }

   for (int i = 0; builtin_str[i] != NULL; i++) {
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
    char hostname[1024]; // hostname buffer
    char *home_directory;
    char *relative_path;

    hostname[1023] = '\0'; // Ensure null termination
    gethostname(hostname, 1023); // Get the hostname

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
        // attempt to abbreviate home directory
        home_directory = getenv("HOME");
        if (home_directory && strncmp(cwd, home_directory, strlen(home_directory)) == 0) {
            // If cwd contains the home directory, replace it with '~' for abbreviation
            int len = strlen(cwd) - strlen(home_directory) + 2; 
            relative_path = malloc(len);
            if (relative_path == NULL) {
                perror("lsh: unable to allocate memory");
                exit(EXIT_FAILURE);
            }
            //relative path starts with ~
            snprintf(relative_path, len, "~%s", cwd + strlen(home_directory));
        } else {
            // Allocate and copy the cwd as is if it doesn't start with home_director
            relative_path = strdup(cwd);
            if(relative_path == NULL) {
              perror("lsh: unable to allocate memory");
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
        args = lsh_split_line(line);
        
        // Execute command
        status = lsh_execute(args);

        // Free memory
        free(line);
        free(args);
        free(relative_path);
    } while (status);
}

//main loop
int main(int argc, char **argv)
{
//call readline
initialize_readline();
//call command loop
lsh_loop();
//shutdown/cleanup
return EXIT_SUCCESS;
}
