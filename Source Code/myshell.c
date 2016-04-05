#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h> 
#include <string.h>

#include "myshell.h"
#include "parser.h"
#include "commands.h"

// Handles signals like ctrl-C
void sigint_handler(int signum) {

	// To handle ctrl-C
	LOG_YELLOW("\nStopping all processes\n");
	LOG_YELLOW("Type exit to terminate, enter to continue\n");
}

// Starts the main loop for myshell
void myshell_loop() {
	char *line;
	char **args;
  	int status = TRUE;
  	COMMAND_TYPE cmd_type;
  
  	do {
  		myshell_print_prompt();
    	line = myshell_read_line();

    	// To handle ctrl-D
    	if(strlen(line) == 0) {
    		LOG_RED("You pressed Ctrl-D\n");
    		myshell_print_bye_message();
    		exit(EXIT_FAILURE);
    	}
    	status = myshell_execute_line(line);
  	} while (status);
}

// Print welcome message for the shell
void myshell_print_welcome_message() {
	printf("-------------------------------------------------------------------------------------------------\n");
  	LOG_GREEN("Welcome to Simple Shell 1.0 - myshell\n");
}

// Print exit message for the shell
void myshell_print_bye_message() {
	LOG_RED("Logging out\n");
  	LOG_RED("Thank you for using Simple Shell 1.0 - myshell\n");
  	printf("-------------------------------------------------------------------------------------------------\n");
}

// Display shell's prompt
void myshell_print_prompt() {
	char cwd[BUFFER_SIZE];

	LOG_BLUE("myshell 1.0 ");
    getcwd(cwd, sizeof(cwd));
    LOG_PINK(cwd);
   	LOG_BLUE(" > ");
}

// Read a single line completely from the input
char* myshell_read_line() {
	char *line = NULL;
	size_t bufsize = 0; 
  	getline(&line, &bufsize, stdin);
  	return line;
}

// Shell starts here
int main(int argc, char **argv, char ** envp) {

	myshell_print_welcome_message();

	environ = envp;
  	
  	if(signal(SIGINT, sigint_handler) == SIG_ERR) {
		LOG_RED("Failed to set up signal handler");
		myshell_print_bye_message();
	}

  	myshell_loop();

  	return EXIT_SUCCESS;
}
