//
//  shell.h
//  yxsh
//
//  Created by Кирилл on 14.03.2018.
//  Copyright © 2018 Кирилл. All rights reserved.
//

#ifndef _SHELL_H
#define _SHELL_H

#include <stdbool.h>

#define MAXARGS 256
#define MAXCMDS 64

struct command {
  char flags;
  char* cmdargs[MAXARGS];
  char* infile;
  char* outfile;
};

typedef struct command command_t;

struct commandline {
  command_t cmds[MAXCMDS];
};

typedef struct commandline commandline_t;

/*  Command flag's  */
#define FLAG_IN_PIPE 1
#define FLAG_OUT_PIPE 2
#define FLAG_BACKGROUND 4
#define FLAG_APPLY_FILE 8
#define FLAG_MERGE_OUT 16

#endif /* _SHELL_H */

