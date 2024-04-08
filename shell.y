
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD
%token NOTOKEN 
%token GREAT 
%token NEWLINE 
%token LESS 
%token GREATGREAT
%token PIPE
%token AMPERSAND 
%token GREATAMP
%token GREATGREATAMP

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include <stdlib.h>
#include "command.h"
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;
command: simple_command
        ;

simple_command:	
        pipe iomodifier_opt AMPERSAND NEWLINE {
		printf("   Yacc: insert background = TRUE\n");
		Command::_currentCommand._background=1;
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| pipe iomodifier_opt NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	
	| NEWLINE 
	| error NEWLINE { yyerrok; }
	;

pipe: 
	pipe PIPE command_and_args {
		printf("   Yacc: Pipe commands\n");
	}
	| command_and_args
	;
command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

iomodifier_opt:
	GREAT WORD {
			printf("   Yacc: insert output \"%s\"\n", $2);
		        Command::_currentCommand._outFile = $2;
	}
	| GREATGREAT WORD {
			printf("   Yacc: append output \"%s\"\n", $2);
			Command::_currentCommand._append = 1;
		        Command::_currentCommand._outFile = $2;
	}
	| LESS WORD {
			printf("   Yacc: take input \"%s\"\n", $2);
			Command::_currentCommand._inputFile = $2;
	}
	| GREATAMP WORD {
			printf("   Yacc: insert error file \"%s\"\n", $2);
		 	Command::_currentCommand._errFile = $2;
	}
	| GREATGREATAMP WORD {
			printf("   Yacc: append error file \"%s\"\n", $2);
			Command::_currentCommand._append = 1;
		    	Command::_currentCommand._errFile = $2;
	}
	| LESS WORD GREAT WORD {
			printf("   Yacc: insert input \"%s\"\n", $2);
		        Command::_currentCommand._inputFile = $2;
		        printf("   Yacc: insert output \"%s\"\n", $4);
		        Command::_currentCommand._outFile = $4;
	}
	| GREAT WORD LESS WORD {
			printf("   Yacc: insert input \"%s\"\n", $4);
		        Command::_currentCommand._inputFile = $4;
		        printf("   Yacc: insert output \"%s\"\n", $2);
		        Command::_currentCommand._outFile = $2;
	}
	| LESS WORD GREATGREAT WORD {
			printf("   Yacc: insert input \"%s\"\n", $2);
		        Command::_currentCommand._inputFile = $2;
		        printf("   Yacc: insert output \"%s\"\n", $4);
		        Command::_currentCommand._append = 1;
		        Command::_currentCommand._outFile = $4;
	}
	|
%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif