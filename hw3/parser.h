/*
 * parser.h
 *
 * Description:
 * This program is a header file containing the components that make up
 * the parser functionalities.
 */

#ifndef PARSER_H
#define PARSER_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define TOKEN_DELIMITERS "&%<>"
#define COMMAND_DELIMITERS "\n\0;|"
#define MAXLEN 100
#define COMMAND_BUFSIZE 1024

char *read_line();
char ***parse_command(char *line, int *num_commands);

#endif