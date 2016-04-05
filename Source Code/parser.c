#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termcap.h>
#include <dirent.h> 
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>
#include <regex.h>

#include "parser.h"
#include "commands.h"

// Determines the type of command
int myshell_execute_line(char *line) {
	COMMAND_TYPE cmd_type;
	Command *cmd = NULL, *cmd2 = NULL;
	int status = FALSE;

	cmd = (Command*)malloc(sizeof(Command)); 
	if(!cmd) {
		LOG_RED("Myshell: malloc() error\n");
		exit(EXIT_FAILURE);
	}

	cmd2 = (Command*)malloc(sizeof(Command)); 
	if(!cmd2) {
		LOG_RED("Myshell: malloc() error\n");
		exit(EXIT_FAILURE);
	}

	cmd_type = myshell_get_command_type(line);

	switch(cmd_type) {
		case INVALID : status = TRUE;
					   break;

		case BACKGROUND : myshell_parse_bg(line, cmd);
						  if(cmd == NULL || cmd -> type == INVALID)	{
						      LOG_YELLOW("Warning: Myshell does not support the operation\n");
							  status = TRUE;
						  } 
						  else {
							  status =  myshell_run_bg(cmd);
						  }
						  break;

		case PIPE : myshell_parse_pipe(line, cmd, cmd2);
					if(cmd == NULL || cmd2 == NULL || cmd->type == INVALID || cmd2->type == INVALID) {
						LOG_YELLOW("Warning: Myshell does not support the operation\n");
						status = TRUE;
					} 
					else {
						status = myshell_run_pipe(cmd, cmd2);
					}
					break;

		case INPUT_REDIRECTION: myshell_parse_redirection(line, cmd, INPUT_REDIRECTION);
								if(cmd == NULL || cmd->type == INVALID) {
									LOG_YELLOW("Warning: Myshell does not support the operation\n");
									status = TRUE;
								}
								else {
									status = myshell_run_redirection(cmd, INPUT_REDIRECTION);
								}
								break;

		case OUTPUT_REDIRECTION: myshell_parse_redirection(line, cmd, OUTPUT_REDIRECTION);
								 if(cmd == NULL || cmd->type == INVALID) {
									LOG_YELLOW("Warning: Myshell does not support the operation\n");
									status = TRUE;
								}
								else {
									status = myshell_run_redirection(cmd, OUTPUT_REDIRECTION);
								}
								break;

		case OTHER: myshell_parse_command(line, cmd);
					if(cmd == NULL || cmd->type == INVALID) {
						LOG_YELLOW("Warning: Myshell does not support the operation\n");
						status = TRUE;
					}
					else {
						status = myshell_run_command(cmd);
					}
					break;

		default : status = TRUE;
	}

	free(cmd);
	free(cmd2);
	return status;
}

// Parse and create struct for command that is Background
void myshell_parse_bg(char* line, Command* cmd) {
	char** tokens, ** final_tokens;

	tokens = myshell_split_line(line, "&");
	final_tokens = myshell_split_line(tokens[0], MYSHELL_TOK_DELIM);
	cmd->name = final_tokens[0];
	cmd->args = final_tokens;
	cmd->type = BACKGROUND;

	final_tokens = myshell_split_line(tokens[1], MYSHELL_TOK_DELIM);
	if(!final_tokens)
		cmd->type = INVALID;
}

// Parse and create struct for command that is Pipe
void myshell_parse_pipe(char* line, Command* cmd1, Command* cmd2) {
	char** tokens, ** final_tokens;

	tokens = myshell_split_line(line, "|");
	if(tokens[0] == NULL || tokens[1] == NULL) {
		cmd1->type = INVALID;
		cmd2->type = INVALID;
	}
	else {
		final_tokens = myshell_split_line(tokens[0], MYSHELL_TOK_DELIM);
		cmd1->name = final_tokens[0];
		cmd1->args = final_tokens;
		cmd1->type = PIPE;

		final_tokens = myshell_split_line(tokens[1], MYSHELL_TOK_DELIM);
		cmd2->name = final_tokens[0];
		cmd2->args = final_tokens;
		cmd2->type = PIPE;
	}
}

// Parse and create struct for command that is I/O Redirection
void myshell_parse_redirection(char* line, Command* cmd, COMMAND_TYPE type)	{
	char** tokens, ** final_tokens;
	char delimiter[BUFFER_SIZE];

	if(type == INPUT_REDIRECTION)
		strcpy(delimiter, "<");
	else if(type == OUTPUT_REDIRECTION)
		strcpy(delimiter, ">");

	tokens = myshell_split_line(line, delimiter);
	if(tokens[0] == NULL || tokens[1] == NULL) {
		cmd->type = INVALID;
	}
	else {
		final_tokens = myshell_split_line(tokens[0], MYSHELL_TOK_DELIM);
		cmd->name = final_tokens[0];
		cmd->args = final_tokens;
		cmd->type = type;

		final_tokens = myshell_split_line(tokens[1], MYSHELL_TOK_DELIM);
		strcpy(cmd->filename, final_tokens[0]); 
	}
}

// Parse and create struct for command that is BUILTIN or EXTERNAL
void myshell_parse_command(char* line, Command* cmd) {
	char **tokens;
	tokens = myshell_split_line(line, MYSHELL_TOK_DELIM);

	cmd->name = tokens[0];
	cmd->args = tokens;

	if(is_builtin_cmd(cmd->name))
		cmd->type = BUILTIN;
	else
		cmd->type = OTHER;
}

// Splits a line into tokens based on delimiter
char** myshell_split_line(char *line, char* delimiter) {
  int position = 0;
  char **tokens = malloc(BUFFER_SIZE * sizeof(char*));
  char *token;

  if (!tokens) {
    LOG_RED("Myshell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, delimiter);
  while (token != NULL) {
    tokens[position] = token;
    position++;
    token = strtok(NULL, delimiter);
  }

  tokens[position] = NULL;
  return tokens;
}

// Matches regualr expression in the line
int match_regexp(char* line, char* exp) {
	regex_t regex;
    int reti;
    int isMatch = FALSE;
    char msgbuf[BUFFER_SIZE];

    // Compile regular expression
    reti = regcomp(&regex, exp, 0);
    if(reti) { 
    	LOG_RED("Could not compile regex\n");
    	return FALSE;
    }

    // Execute regular expression
    reti = regexec(&regex, line, 0, NULL, 0);
    if(!reti) {
    	isMatch = TRUE;
    }
    else if(reti == REG_NOMATCH) {
    	isMatch = FALSE;
    }
    else { 
    	regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        isMatch = FALSE;
    }
    regfree(&regex);

    return isMatch;
}
