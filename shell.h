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
#define MAXCMDS 50

struct command {
  char* cmdargs[MAXARGS];
  char cmdflag;
};

typedef struct command command_t;

struct commandline {
  command_t cmds[MAXCMDS];
  char* infile;
  char* outfile;
  char* appfile;
  bool background;
};

typedef struct commandline commandline_t;

/*  cmdflag's  */
#define OUTPIP 01
#define INPIP 02

#endif /* _SHELL_H */
