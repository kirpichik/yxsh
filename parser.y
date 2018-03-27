%{

#define YYSTYPE char *

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

int yylex(void);
void yyerror(const char* s);
void yy_scan_string(char* str);

%}

%token WORD
%token NEWLINE
%token PIPE
%token BACKGROUND
%token SEPARATOR
%token IN_FILE
%token OUT_FILE
%token ADD_FILE


%start  complete_command

%%

complete_command : list separator { YYACCEPT; }
                 | list           { YYACCEPT; }
                 | NEWLINE        { YYACCEPT; }
                 ;

list             : list separator_op pipeline
                 |                   pipeline
                 ;

pipeline         :                         command
                 | pipeline PIPE linebreak command { printf("PIPE "); }
                 ;

command          : WORD cmd_suffix { printf("Command: %s ", $1); }
                 | WORD { printf("Command: %s ", $1); }
                 ;

cmd_suffix       :            io_file
                 | cmd_suffix io_file
                 |            WORD { printf("ARG: %s ", $1); }
                 | cmd_suffix WORD { printf("ARG: %s ", $2); }
                 ;

io_file          : IN_FILE  WORD { printf("IN: %s ", $2);  }
                 | OUT_FILE WORD { printf("OUT: %s ", $2); }
                 | ADD_FILE WORD { printf("ADD: %s ", $2); }
                 ;

linebreak        : NEWLINE
                 | /* empty */
                 ;

separator_op     : BACKGROUND { printf("BGKGRND "); }
                 | SEPARATOR
                 ;

separator        : separator_op linebreak
                 | NEWLINE
                 ;

%%

void yyerror(const char* s) {
  printf("Parse error.");
}

int yywrap() {
  return 1;
}

int parseline(char* line, commandline_t* cmds) {
  yy_scan_string(line);
  return 0;
}

/*int main(void) {
  yyparse();
  return yyparse();
}*/

