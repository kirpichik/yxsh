//
//  main.cpp
//  yxsh
//
//  Created by Кирилл on 14.03.2018
//  Copyright © 2018 Кирилл. All rights reserved.
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "shell.h"

#include "parser.h"
#include "promptline.h"

#define PROMPT "[yx!]> "

int main(int argc, char* argv[]) {
  commandline_t commandline;
  char line[1024]; // allow large command lines
  int ncmds;

  /* PLACE SIGNAL CODE HERE */

  while (promptline(PROMPT, line, sizeof(line)) > 0) { // until eof
    if ((ncmds = parseline(line, &commandline)) <= 0)
      continue; // read next line
#ifdef DEBUG
    {
      for (int i = 0; i < ncmds; i++) {
        for (int j = 0; commandline.cmds[i]->cmdargs[j] != (char*)NULL; j++)
          fprintf(stderr, "cmd[%d].cmdargs[%d] = %s\n", i, j,
                  commandline.cmds[i]->cmdargs[j]);
        fprintf(stderr, "cmds[%d].cmdflag = %o\n", i, commandline.cmds[i]->cmdflag);
      }
    }
#endif

    for (int i = 0; i < ncmds; i++) {
      pid_t pid = fork();
      if (pid == 0) {
        if (execvp(commandline.cmds[i].cmdargs[0], commandline.cmds[i].cmdargs))
          perror("Cannot execute");
      }
      wait(NULL);
    }

  } // close while
  return 0;
}

/* PLACE SIGNAL CODE HERE */

