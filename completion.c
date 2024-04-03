#include "completion.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <limits.h> // For PATH_MAX
#include <glob.h>

//added manual def of PATH_MAX
#ifndef PATH_MAX
#define PATH_MAX 4096 
#endif

//use array from another file
extern char *builtin_str[]; 

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

