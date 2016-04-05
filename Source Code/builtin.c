#include "commands.h"

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

// exit - quits the shell
int myshell_exit(Command *cmd) {
	myshell_print_bye_message();
	exit(EXIT_SUCCESS);
	return TRUE;
}

// clr - clears the screen
int myshell_clr(Command *cmd) {
	char buf[1024];
  	char *str;

  	tgetent(buf, getenv("TERM"));
  	str = tgetstr("cl", NULL);
  	fputs(str, stdout);

	return TRUE;
}

// environ - prints the env variables 
int myshell_environ(Command *cmd) {
	  char **env_aux;
    if(cmd->args[1] != NULL)
      return TRUE;
  	for(env_aux = environ; *env_aux != 0; env_aux ++) {
    	printf("%s\n", *env_aux);
    }
	return TRUE;
}

// pause - pauses shell until enter is pressed
int myshell_pause(Command *cmd) {
	getpass("Press Enter to continue...");
	return TRUE;
}

// help - displays user manual
int myshell_help(Command *cmd) {
	char readme_path[1024];
  if(cmd->args[1] != NULL)
      return TRUE;
  strcpy(readme_path, "usermanual");
  system("more usermanual");
	return TRUE;
}

// echo - displays message
int myshell_echo(Command *cmd) {
  	int i = 1;

  	while(cmd->args[i]) {
    	printf("%s ", cmd->args[i]);
    	i++;
  	}
  	printf("\n");

	return TRUE;
}

// cd - changes directory and sets PWD
int myshell_cd(Command *cmd) {
	char cwd[1024];
  	char path[1024];
  	char *home = NULL;

  	if(cmd->args[1] == NULL) {
    	getcwd(cwd,sizeof(cwd));
    	fprintf(stdout, "No argument specified! In current directory: %s\n", cwd);
  	} 
  	else {
    	sprintf( path, "%s", cmd->args[1] );
      	if(cmd->args[1][0] == '~') {
        	home = getenv( "HOME" );
        	if ( home == NULL ) {
          		fprintf(stdout, "cd: couldn't reslove ~\n" );
        	}
        	sprintf( path, "%s%*s", home, (int)(strlen(cmd->args[1])-1), cmd->args[1]+1 );
     	}

  		if( chdir( path ) != 0 ) {
    		switch( errno ) {
				case EACCES: fprintf(stdout, "cd: %s: Search permission denied\n", path );
            			 	 break;
            
        		case ELOOP:	 fprintf(stdout,"cd: %s: Loop in path\n", path );
            			     break;
            
        		case ENAMETOOLONG: fprintf(stdout, "cd: %s: Length of path is too long\n",path);
            	               	   break;
            
        		case ENOENT: fprintf(stdout, "cd: %s: No such file or directory\n", path );
            			     break;
            
        		case ENOTDIR: fprintf(stdout,"cd: %s: Part of path not a directory\n", path );
            			  	  break;
        	}
    	}
		else {
    		getcwd(cwd,sizeof(cwd));
    		setenv("PWD", cwd, 1);
    	}
    }
  	return TRUE;
}

// dir - lists the contents of specified directory
int myshell_dir(Command *cmd) {
	DIR *mydir;
    struct dirent *myfile;
    struct stat mystat;
    char buf[BUFFER_SIZE];

    if(cmd->args[1] == NULL) {
      	fprintf(stdout, "No directory specified\n");
    } 
    else {
    	mydir = opendir(cmd->args[1]);
    	if(mydir != NULL) {
      		LOG_CYAN("Size in bytes\t\tName\n");

    		while((myfile = readdir(mydir)) != NULL) {
        		sprintf(buf, "%s/%s", cmd->args[1], myfile->d_name);
        		stat(buf, &mystat);
        		fprintf(stdout, "%-23lld %-15s\n",mystat.st_size, myfile->d_name);
    		}
    		closedir(mydir);
  		}
  		else {
  			switch( errno ) {

            	case EACCES: fprintf(stdout, "dir: %s: Permission denied\n", cmd->args[1]);
            			 	 break;
   
        		case ENOENT: fprintf(stdout, "dir: %s: No such file or directory\n", cmd->args[1]);
            		  	 	 break;
            
        		case ENOTDIR: fprintf( stdout,"dir: %s: Part of path not a directory\n", cmd->args[1]);
            			  	  break;

        		default: fprintf( stdout,"dir: %s: Unknown error occurred\n", cmd->args[1]);
          	}
    	
  		}
  	}
	return TRUE;
}