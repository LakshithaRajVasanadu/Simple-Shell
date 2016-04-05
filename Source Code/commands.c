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

#include "commands.h"
#include "parser.h"

// List of built in commands
char *builtin_str[] = {
  "cd", 
  "environ", 
  "exit", 
  "clr", 
  "pause", 
  "help", 
  "echo", 
  "dir"
};

// Map of builtin commands - function
int (*builtin_func[]) (Command *) = {
  &myshell_cd, 
  &myshell_environ, 
  &myshell_exit, 
  &myshell_clr, 
  &myshell_pause, 
  &myshell_help, 
  &myshell_echo, 
  &myshell_dir
};

// Returns the number of built in commands
int myshell_num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}

// Checks whether a given command is builtin or not
int is_builtin_cmd(char *cmd) {
	int i;
	for(i = 0; i < myshell_num_builtins(); i++) {
    	if (strcmp(cmd, builtin_str[i]) == 0) {
        	return TRUE;
        }
    }
    return FALSE;
}

// Returns command type for a line of input
COMMAND_TYPE myshell_get_command_type(char *line) {
	if(match_regexp(line, "^[ \t\r\n\v\f]*$")) {
		return INVALID;
	}

	if( (match_regexp(line, "&") && (match_regexp(line, "[|<>]")))
		|| (match_regexp(line, ">") && (match_regexp(line, "[&|<]")))
		|| (match_regexp(line, "<") && (match_regexp(line, "[&|>]")))
		|| (match_regexp(line, "|") && (match_regexp(line, "[&<>]"))) )	{
		LOG_YELLOW("Warning: Myshell does not support the operation\n");
		return INVALID;
	}

	if( (match_regexp(line, "&")) ) {
		if( match_regexp(line, "&[ \t\r\n\v\f]*$") )
			return BACKGROUND;
	}
	
	if(match_regexp(line, "|"))
		return PIPE;

	if(match_regexp(line, "<"))
		return INPUT_REDIRECTION;

	if(match_regexp(line, ">"))
		return OUTPUT_REDIRECTION;

	return OTHER;
}

// Runs Background command
int myshell_run_bg(Command* cmd) {
	int pid = fork();
	if(pid < 0) {
    	LOG_RED("Myshell: fork() error\n");
    	return FALSE;
  	} 
  	else if (pid == 0) {
    	execvp(cmd->name, cmd->args);
    	return FALSE;
  	} 
  	else {
    	//Parent doesn't wait for the child to finish
  	}
  return TRUE;
}

// Runs Piped commands
int myshell_run_pipe(Command* cmd1, Command* cmd2) {
	int status = 0;
  	int fd[2];
  	int pid, pid2, wpid;

    // Sets up a pipeline
  	if (pipe(fd) == -1) {
    	LOG_RED("Myshell: pipe() error\n");
    	return FALSE;
  	}

  	if ((pid = fork()) < 0) {
    	LOG_RED("Myshell: fork() error\n");
    	return FALSE;
  	}
	
	if (pid == 0) {
    	close(fd[STDOUT]); // Close the other side of pipe
    	dup2(fd[STDIN], STDIN);  // Automatically closes previous fd[0]
    	close(fd[STDIN]);  // cleanup
    	execvp(cmd2->name, cmd2->args);
    	return FALSE;
  	} 
  	else {
    	// parent process need to fork again, so the shell isn't replaced
    	if ((pid2 = fork()) < 0) {
      		LOG_RED("Myshell: fork() error\n");
    		  exit(EXIT_FAILURE);
    	}
    	if (pid2 == 0) {
      		close(fd[STDIN]);
      		dup2(fd[STDOUT], STDOUT);
      		close(fd[STDOUT]);
      		execvp(cmd1->name, cmd1->args);
      		return FALSE;
    	} 
    	else {
      		// parent process (the shell)
      		close(fd[STDIN]);
      		close(fd[STDOUT]);
      		do {
      			wpid = waitpid(pid2, &status, WUNTRACED);
    		}while (!WIFEXITED(status) && !WIFSIGNALED(status));
    	}
  	}
  	sleep(1);
  	return TRUE;
}

// Runs I/O redirection commands
int myshell_run_redirection(Command* cmd, COMMAND_TYPE type)	{
	int pid, wpid;
	FILE* file;
	int status = 0;

  if((pid = fork()) < 0) {
    LOG_RED("Myshell: fork() error\n");
    return FALSE;
  }
  
	if(pid == 0) {
		if(type == INPUT_REDIRECTION) {
    		if(freopen(cmd->filename, "r", stdin) == NULL) {
    			LOG_RED("Input redirection failed\n");
    			return TRUE;
    		}
    	} 
    	else if(type == OUTPUT_REDIRECTION) {
    		if(freopen(cmd->filename, "w", stdout) == NULL) {
    			LOG_RED("Output redirection failed\n");
    			return TRUE;
    		}
    	}
    	execvp(cmd->name, cmd->args);
    	return FALSE;
 	}
 	else {
  		do {
      		wpid = waitpid(pid, &status, WUNTRACED);
    	}while (!WIFEXITED(status) && !WIFSIGNALED(status));
  	}
	return TRUE;
}

// Runs other types of commands - builtin / other
int myshell_run_command(Command* cmd) {
	if(cmd->type == BUILTIN)
		return myshell_run_builtin(cmd);
	else
		return myshell_run_other(cmd);
}

// Selects the matching builtin command function to be executed
int myshell_run_builtin(Command* cmd) {
	int i;
	for(i = 0; i < myshell_num_builtins(); i++) {
    	if (strcmp(cmd->name, builtin_str[i]) == 0) {
        	return (*builtin_func[i])(cmd);
		}
	}
	return TRUE;
}

// Runs external commands
int myshell_run_other(Command* cmd) {
	pid_t pid, wpid;
  	int status;

  	pid = fork();
  	if (pid == 0) {
  		if(strcmp(cmd->name, "ls") == 0) {
  			myshell_ls(cmd);	// Own implementation of ls and ls -l
  		}
    	else {
    		if (execvp(cmd->name, cmd->args) == -1) {
      			LOG_RED("Myshell: No such operation\n");
      		}
    	}
    	exit(EXIT_FAILURE);
  	} 
  	else if (pid < 0) {
    	LOG_RED("Myshell: fork() error\n");
  	} 
  	else {
    	do {
      		wpid = waitpid(pid, &status, WUNTRACED);
    	}while (!WIFEXITED(status) && !WIFSIGNALED(status));
  	}
	return TRUE;
}

// Helper function to find the file type for ls
int filetypeletter(int mode) {
    char c;
	
	if (S_ISREG(mode))
        c = '-';
    else if (S_ISDIR(mode))
        c = 'd';
    else if (S_ISBLK(mode))
        c = 'b';
    else if (S_ISCHR(mode))
        c = 'c';
#ifdef S_ISFIFO
    else if (S_ISFIFO(mode))
        c = 'p';
#endif  /* S_ISFIFO */
#ifdef S_ISLNK
    else if (S_ISLNK(mode))
        c = 'l';
#endif  /* S_ISLNK */
#ifdef S_ISSOCK
    else if (S_ISSOCK(mode))
        c = 's';
#endif  /* S_ISSOCK */
#ifdef S_ISDOOR
    else if (S_ISDOOR(mode))
        c = 'D';
#endif  /* S_ISDOOR */
    else
    {
        /* Unknown type -- possibly a regular file? */
        c = '?';
    }
    return(c);
}

// Convert a mode field into "ls -l" type perms field.
char* lsperms(int mode)
{
    static const char *rwx[] = {"---", "--x", "-w-", "-wx",
    "r--", "r-x", "rw-", "rwx"};
    static char bits[11];

    bits[0] = filetypeletter(mode);
    strcpy(&bits[1], rwx[(mode >> 6)& 7]);
    strcpy(&bits[4], rwx[(mode >> 3)& 7]);
    strcpy(&bits[7], rwx[(mode & 7)]);
    if (mode & S_ISUID)
        bits[3] = (mode & S_IXUSR) ? 's' : 'S';
    if (mode & S_ISGID)
        bits[6] = (mode & S_IXGRP) ? 's' : 'l';
    if (mode & S_ISVTX)
        bits[9] = (mode & S_IXUSR) ? 't' : 'T';
    bits[10] = '\0';
    return(bits);
}

// Implementation of ls and ls -l
int myshell_ls(Command *cmd) {
	DIR *mydir;
  	struct dirent *myfile;
  	struct stat mystat;
  	char buf[BUFFER_SIZE];
  	char cwd[BUFFER_SIZE];
  	int with_option = FALSE;
  	char date[BUFFER_SIZE];
  	long total = 0;

  	if(cmd->args[1] == NULL) {
    	with_option = FALSE;
  	}
  	else if(!strcmp(cmd->args[1], "-l")) {
    	with_option = TRUE;
  	}
  	else {
    	fprintf(stdout, "ls: Argument not supported\n");
    	return TRUE;
  	}
	
	getcwd(cwd,sizeof(cwd));
  	mydir = opendir(cwd);

  	if(with_option == TRUE) {
    	LOG_CYAN("Permissions\tLinks\tUser\t\tGroup\tSize\tLast Modified\tName\n");
  	}

  	if(mydir != NULL) {
    	while((myfile = readdir(mydir)) != NULL) {
        	if(!strcmp(myfile->d_name, ".") || !strcmp(myfile->d_name, "..") || myfile->d_name[0] == '.')
          		continue;

        	if(with_option == FALSE) {
          		printf("%-15s\n",myfile->d_name);
        	} 
        	else if(with_option == TRUE) {
          		sprintf(buf, "%s/%s", cwd, myfile->d_name);
          		stat(buf, &mystat);

          		printf("%s\t",lsperms(mystat.st_mode));
          		printf("%d\t", mystat.st_nlink);
          		printf("%s\t", (getpwuid(mystat.st_uid))->pw_name);
          		printf("%s\t", (getgrgid(mystat.st_gid))->gr_name);
          		printf("%lld\t", mystat.st_size);
          		strftime(date, 20, "%b %d %R", localtime(&mystat.st_mtime));
          		printf("%s\t", date);
          		printf("%-15s\n",myfile->d_name);
          
          		total += mystat.st_blocks;	
        	}
		}

    	if(with_option == TRUE) {
      		printf("total %ld\n", total);
    	}

    	closedir(mydir);
  	} 
  	else {
    	switch( errno ) {
			case EACCES:
				fprintf(stderr, "ls: %s: Permission denied\n", cwd );
            	break;

        	case ENOENT:
				fprintf(stderr, "ls: %s: No such file or directory\n", cwd );
            	break;
            
        	case ENOTDIR:
				fprintf( stderr,"ls: %s: Part of path not a directory\n", cwd );
            	break;
        
        	default:
            	fprintf( stderr,"ls: %s: Unknown error occurred\n", cwd );
        }	
  	}
  	
  	return TRUE;
}