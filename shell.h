//
//  shell.h
//  yxsh
//
//  Created by Кирилл on 14.03.2018.
//  Copyright © 2018 Кирилл. All rights reserved.
//

#ifndef _SHELL_H
#define _SHELL_H

#define MAXARGS 256
#define MAXCMDS 50

struct command {
  char* cmdargs[MAXARGS];
  char cmdflag;
};

/*  cmdflag's  */
#define OUTPIP 01
#define INPIP 02

extern struct command cmds[];
extern char *infile, *outfile, *appfile;
extern char bkgrnd;

int parseline(char*);
int promptline(char*, char*, int);

#endif /* _SHELL_H */
