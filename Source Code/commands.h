#ifndef _COMMANDSH_
#define _COMMANDSH_

#include "myshell.h"

#define MYSHELL_TOK_DELIM " \t\r\n\a"

// Command types supported
typedef enum command_type_t {
	BUILTIN,
	BACKGROUND,
	INPUT_REDIRECTION,
	OUTPUT_REDIRECTION,
	PIPE,
	OTHER,
	INVALID
}COMMAND_TYPE;

// Definition of command
struct command_t {
	char* name;
	char** args;
	char filename[BUFFER_SIZE];
	COMMAND_TYPE type;
};

typedef struct command_t Command;

// Returns the number of builtin commands supported
int myshell_num_builtins();

// Checks if a given command is builtin or not
int is_builtin_cmd(char* cmd);

// Returns the command type for input line
COMMAND_TYPE myshell_get_command_type(char *line);

// Running various types of commands
int myshell_run_bg(Command* cmd);
int myshell_run_pipe(Command* cmd1, Command* cmd2);
int myshell_run_redirection(Command* cmd, COMMAND_TYPE type);
int myshell_run_command(Command* cmd);
int myshell_run_builtin(Command* cmd);
int myshell_run_other(Command* cmd);

// Implementation of ls and ls -l
int myshell_ls(Command* cmd);

//Built in commands
int myshell_cd(Command *cmd);
int myshell_environ(Command *cmd);
int myshell_exit(Command *cmd);
int myshell_clr(Command *cmd);
int myshell_pause(Command *cmd);
int myshell_help(Command *cmd);
int myshell_echo(Command *cmd);
int myshell_dir(Command *cmd);

#endif
