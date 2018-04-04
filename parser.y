%{

#define YYSTYPE char*

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parseline.h"

static char* current_buff;
static commandline_t* current_cmds;
static int commands_count;

static int args_count;
static command_t current_cmd;

/**
 * Commands for bison.
 */
static int yylex(void);
static void yyerror(const char*);

/**
 * Prepares current_cmd structure for next command.
 */
static void prepare_temp_command();

/**
 * Stores arg with current command.
 *
 * @param word Arg word.
 *
 * @return Non-zero if to many args.
 */
static int storeArg(char* word);

/**
 * Stores current command.
 *
 * @param name Command name.
 *
 * @return Non-zero if to many commands.
 */
static int storeCommand(char* name);

/**
 * Flags previous commands with pipe.
 */
static void pipeCommands();

/**
 * Flags previous command with background.
 */
static void backgroundCommand();

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
                 | pipeline PIPE linebreak command { pipeCommands(); }
                 ;

command          : WORD cmd_suffix { storeCommand($1); }
                 | WORD { storeCommand($1); }
                 ;

cmd_suffix       :            io_file
                 | cmd_suffix io_file
                 |            WORD { storeArg($1); }
                 | cmd_suffix WORD { storeArg($2); }
                 ;

io_file          : IN_FILE  WORD { current_cmd.infile = $2;                                        }
                 | OUT_FILE WORD { current_cmd.outfile = $2;                                       }
                 | ADD_FILE WORD { current_cmd.outfile = $2; current_cmd.flags |= FLAG_APPLY_FILE; }
                 ;

linebreak        : NEWLINE
                 | /* empty */
                 ;

separator_op     : BACKGROUND { backgroundCommand(); }
                 | SEPARATOR
                 ;

separator        : separator_op linebreak
                 | NEWLINE
                 ;

%%

static void yyerror(const char* s) {
  printf("yxsh: Syntax error.\n");
}

int parseline(char* line, commandline_t* cmds) {
  current_buff = line;
  current_cmds = cmds;
  prepare_temp_command();
  commands_count = 0;
  
  if (yyparse())
    return -1;

  return commands_count;
}

static void pipeCommands() {
  current_cmds->cmds[commands_count - 1].flags |= FLAG_IN_PIPE;
  current_cmds->cmds[commands_count - 2].flags |= FLAG_OUT_PIPE;
}

static void backgroundCommand() {
  current_cmds->cmds[commands_count - 1].flags |= FLAG_BACKGROUND;
}

static int storeArg(char* word) {
  if (args_count >= MAXARGS)
    return -1;
  
  current_cmd.cmdargs[args_count++] = word;
  return 0;
}

static int storeCommand(char* name) {
  if (commands_count >= MAXCMDS)
    return -1;
  
  current_cmd.cmdargs[0] = name;
  current_cmd.cmdargs[args_count] = NULL;
  memcpy(&(current_cmds->cmds[commands_count++]), &current_cmd, sizeof(command_t));
  prepare_temp_command();
  return 0;
}

static void prepare_temp_command() {
  current_cmd.flags = 0;
  args_count = 1;
  current_cmd.infile = NULL;
  current_cmd.outfile = NULL;
}

static int yylex() {
  int c = *current_buff;
  int word_len = 0;

  while (c == ' ' || c == '\t') {
    c = *(++current_buff);
  }

  current_buff++;
  switch (c) {
    case '\0':
      return 0;
    case '&':
      return BACKGROUND;
    case ';':
      return SEPARATOR;
    case '\n':
      return NEWLINE;
    case '|':
      return PIPE;
    case '<':
      return IN_FILE;
    case '>':
      if ((c = *current_buff) == '>') {
        current_buff++;
        return ADD_FILE;
      }
      return OUT_FILE;
  }
  current_buff--;

  while (!isspace(c)) {
    word_len++;
    c = *(current_buff + word_len);
  }

  if (!word_len)
    return -1;

  yylval = (char*) malloc((word_len + 1) * sizeof(char));
  memcpy(yylval, current_buff, word_len * sizeof(char));
  current_buff += word_len;
  yylval[word_len] = '\0';
  return WORD;
}

