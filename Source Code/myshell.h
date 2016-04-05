#ifndef _MYSHELLH_
#define _MYSHELLH_

#include <setjmp.h>

// Definitions and macros for colored console output - ANSI colors
#define RED_START "\033[0;31m"
#define GREEN_START "\033[0;32m"
#define YELLOW_START "\033[0;33m"
#define BLUE_START "\033[0;34m" 
#define PINK_START "\033[0;35m"
#define CYAN_START "\033[0;36m"

#define COLOR_END "\33[0m" // To flush the previous settings

#define LOG_RED(X) printf("%s%s%s",RED_START,X,COLOR_END)
#define LOG_GREEN(X) printf("%s%s%s",GREEN_START,X,COLOR_END)
#define LOG_YELLOW(X) printf("%s%s%s",YELLOW_START,X,COLOR_END)
#define LOG_BLUE(X) printf("%s%s%s",BLUE_START,X,COLOR_END)
#define LOG_PINK(X) printf("%s%s%s",PINK_START,X,COLOR_END)
#define LOG_CYAN(X) printf("%s%s%s",CYAN_START,X,COLOR_END)

#define TRUE 1
#define FALSE 0

#define STDIN 0
#define STDOUT 1

#define BUFFER_SIZE 1024

// Store env variables
char** environ; 
sigjmp_buf sigint_buf;

// Function definitions
void sigint_handler(int signum);
void myshell_print_welcome_message();
void myshell_print_bye_message();
void myshell_print_prompt();
char* myshell_read_line();
void myshell_loop();

#endif

