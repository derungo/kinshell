#ifndef PARSE_H
#define PARSE_H

// Assumes commands are split into tokens and places them into an array of strings
char **ksh_split_line(char *line);

#endif