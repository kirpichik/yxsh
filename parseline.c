//
//  promptline.cpp
//  yxsh
//
//  Created by Кирилл on 14.03.2018.
//  Copyright © 2018 Кирилл. All rights reserved.
//

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "shell.h"

#include "parseline.h"

static char* blankskip(register char*);

int parseline(char* line, commandline_t* commandline) {
  int nargs, ncmds;
  char* s;
  char aflg = 0;
  int rval;
  int i;
  static char delim[] = " \t|&<>;\n";

  /* initialize  */
  commandline->background = 0;
  nargs = ncmds = rval = 0;
  s = line;
  commandline->infile = commandline->outfile = commandline->appfile = (char*) NULL;
  commandline->cmds[0].cmdargs[0] = (char*) NULL;
  for (i = 0; i < MAXCMDS; i++)
    commandline->cmds[i].cmdflag = 0;

  while (*s) { // until line has been parsed
    s = blankskip(s); // skip white space
    if (!*s)
      break; // done with line

    // handle <, >, |, &, and ;
    switch (*s) {
      case '&':
        commandline->background++;
        *s++ = '\0';
        break;
      case '>':
        if (*(s + 1) == '>') {
          ++aflg;
          *s++ = '\0';
        }
        *s++ = '\0';
        s = blankskip(s);
        if (!*s) {
          fprintf(stderr, "syntax error\n");
          return (-1);
        }

        if (aflg)
          commandline->appfile = s;
        else
          commandline->outfile = s;
        s = strpbrk(s, delim);
        if (isspace(*s))
          *s++ = '\0';
        break;
      case '<':
        *s++ = '\0';
        s = blankskip(s);
        if (!*s) {
          fprintf(stderr, "syntax error\n");
          return (-1);
        }
        commandline->infile = s;
        s = strpbrk(s, delim);
        if (isspace(*s))
          *s++ = '\0';
        break;
      case '|':
        if (nargs == 0) {
          fprintf(stderr, "syntax error\n");
          return (-1);
        }
        commandline->cmds[ncmds++].cmdflag |= OUTPIP;
        commandline->cmds[ncmds].cmdflag |= INPIP;
        *s++ = '\0';
        nargs = 0;
        break;
      case ';':
        *s++ = '\0';
        ++ncmds;
        nargs = 0;
        break;
      default:
        // a command argument
        if (nargs == 0) // next command
          rval = ncmds + 1;
        commandline->cmds[ncmds].cmdargs[nargs++] = s;
        commandline->cmds[ncmds].cmdargs[nargs] = (char*)NULL;
        s = strpbrk(s, delim);
        if (isspace(*s))
          *s++ = '\0';
        break;
    } // close switch
  } // close while

  /*  error check  */

  /*
   *  The only errors that will be checked for are
   *  no command on the right side of a pipe
   *  no command to the left of a pipe is checked above
   */
  if (commandline->cmds[ncmds - 1].cmdflag & OUTPIP) {
    if (nargs == 0) {
      fprintf(stderr, "syntax error\n");
      return (-1);
    }
  }

  return (rval);
}

static char* blankskip(register char* s) {
  while (isspace(*s) && *s)
    ++s;
  return (s);
}
