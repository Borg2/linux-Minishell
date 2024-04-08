/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <ctime>
#include "command.h"

char labPath[] = "/home/mido/Downloads/lab2/lab_3";
char logFileName[] = "/childLog.txt";
FILE *fp;

void openLogFile()
{
	char path[100];
	strcpy(path, labPath);
	strcat(path, logFileName);
	fp = fopen(path, "a");
}

void closeLogFile()
{
	fclose(fp);
}

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

void newline(char *str, int size)
{
	for(int i = 0; i<size; i++)
	{
		if(str[i] == '\n')
		{
			str[i] = '\0';
			return;
		}
	}
}

void handleSIGCHILD(int sig)
{
	openLogFile();
	flockfile(fp);
	time_t timer = time(NULL);
	tm *timeInfo = localtime((&timer));
	char currentTime[32];
	strcpy(currentTime, asctime(timeInfo));
	newline(currentTime, 32);
	fprintf(fp,"Child terminated at %s \n",currentTime);
	funlockfile(fp);
	closeLogFile();
	signal(SIGCHLD,handleSIGCHILD);
}

void
Command::execute()
{
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) 
	{
		prompt();
		return;
	}
	
	//cd command execution
	if (!strcmp(_simpleCommands[0]->_arguments[0],"cd"))
	{
		int dir;
		if (_simpleCommands[0]->_numberOfArguments == 1)
		{
			dir=chdir(getenv("HOME"));
			printf("Home>");
		}
	 	else
		{
			dir=chdir(_simpleCommands[0]->_arguments[1]);
		}
		if(dir<0)
			perror("cd");
		clear();
		prompt();
		return;
	}
	
	//exit command execution
	if (_numberOfSimpleCommands==1 && !strcmp(_simpleCommands[0]->_arguments[0],"exit"))
	{
	printf("\n\t shell termination\n\n\t goodbye \n\n");
	exit(0);
	}
	
	//setting default input/output
	int default_input = dup(0);
	int default_output = dup(1);
	int default_error = dup(2);
	
	int f_in,f_out,f_err;
	
	if(_inputFile)
	{
		f_in = open(_inputFile,O_RDONLY,0664);
		if (f_in < 0)
		{
			perror("inputFile open error");
			exit(1);
		}
	}
	else
	{
		f_in = dup(default_input);
	}
	
	if(_errFile)
	{
		if(!_append)
			f_err = open(_errFile, O_CREAT|O_WRONLY|O_TRUNC, 0664);
		else
			f_err = open(_errFile, O_CREAT|O_WRONLY|O_TRUNC,0664);
		if(f_err < 0)
		{
			perror("errFile open error");
			exit(1);
		}
	}
	else
	{
		f_err = dup(default_error);
	}
	
	
	for ( int i = 0; i <_numberOfSimpleCommands; i++ )
	{
		//redirect input
		dup2(f_in,0);
		dup2(f_err,2);
		close(f_in);
		
		if (i == _numberOfSimpleCommands - 1)
		{
			if(_outFile)
			{
				if(!_append)
			        f_out = open(_outFile, O_CREAT|O_WRONLY|O_TRUNC, 0664);
			    else
			        f_out = open(_outFile, O_CREAT|O_WRONLY|O_APPEND, 0664);
						    
			    if(f_out < 0)
			    {
			        perror("Outfile open error");
			        exit(1);
			    }
			}
			else if(_errFile)
			{
				if(!_append)
			    	f_err = open(_errFile, O_CREAT|O_WRONLY|O_TRUNC, 0664);
				else
					f_err = open(_errFile, O_CREAT|O_WRONLY|O_TRUNC,0664);
				if(f_err < 0)
				{
					perror("errFile open error");
					exit(1);
				}
			}
			else
			{
				f_out= dup(default_output);
				f_err = dup(default_error);
			}
		}
		else
		{
			int fdpipe[2];
			if ( pipe(fdpipe) == -1) 
			{
			perror( "Pipe error!");
			exit( 2 );
			}
			// Redirect output/input to pipe 
			f_in= fdpipe[0];
			f_out=fdpipe[1];
			//close(fdpipe[0]);
			//close(fdpipe[1]);

			// Redirect err 
			dup2( default_error, 2 );
		}
		
		//redirect output 
		dup2(f_out, 1 );
		close(f_out);
			
		int pid = fork();
		if ( pid == -1 ) 
		{
			perror( "fork");
			exit( 2 );
		}
	
		if (pid == 0) 
		{
			//Child
			//command execution 
			execvp(_simpleCommands[i]->_arguments[0],_simpleCommands[i]->_arguments);

			// exec() is not suppose to return, something went wrong
			perror( "execvp");
			exit(1);
		}	
		else //parent
		{
			signal(SIGCHLD, handleSIGCHILD);
			if(!_background)
			{
				waitpid(pid,NULL,0);
			}
		else 
			printf("percent\n");
		}
		
	}
	

	//restore the default input/output
	dup2(default_input,0);
	dup2(default_output,1);
	dup2(default_error,2);
	// Clear to prepare for next command
	clear();
	// Print new prompt
	prompt();
}

void sigHandler(int sig)
{
	printf("\n");
	Command::_currentCommand.prompt();
}
// Shell implementation
void
Command::prompt()
{
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int 
main()
{
	signal(SIGINT,sigHandler);
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}

