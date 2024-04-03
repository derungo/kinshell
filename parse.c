#include "parse.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h> 

#define ksh_RL_BUFSIZE 1024
#define ksh_TOK_BUFSIZE 64
#define ksh_TOK_DELIM " \t\r\n\a"

//tokenization of input
char **ksh_split_line(char *line)
{
  int bufsize = ksh_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "ksh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, ksh_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += ksh_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "ksh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, ksh_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}