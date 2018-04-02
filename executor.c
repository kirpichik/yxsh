//
//  executor.c
//  yxsh
//
//  Created by Кирилл on 02.04.2018.
//  Copyright © 2018 Кирилл. All rights reserved.
//

#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


#include "executor.h"

void execute(commandline_t* commandline, int ncmds) {
  for (int i = 0; i < ncmds; i++) {
    pid_t pid = fork();
    if (pid == 0) {
      if (execvp(commandline->cmds[i].cmdargs[0], commandline->cmds[i].cmdargs))
        perror("yxsh: Cannot execute");
      exit(0);
    }
    wait(NULL);
  }
}
