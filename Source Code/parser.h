#ifndef _PARSERH_
#define _PARSERH_

#include "commands.h"

// Parses line to create command and then executes it
int myshell_execute_line(char *line);

// Checks if a line matches the regex or not
int match_regexp(char* line, char* exp);

// Tokenize line using delimiter
char** myshell_split_line(char *line, char* delimiter);

// Parsers for different types of commands
void myshell_parse_bg(char* line, Command* cmd);
void myshell_parse_pipe(char* line, Command* cmd1, Command* cmd2);
void myshell_parse_redirection(char* line, Command* cmd, COMMAND_TYPE type);
void myshell_parse_command(char* line, Command* cmd);

#endif

