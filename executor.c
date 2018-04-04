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

/**
 * The work is done with a fork process part.
 */
static void execute_fork(command_t* cmd) {
  if (execvp(cmd->cmdargs[0], cmd->cmdargs))
    perror("yxsh: Cannot execute");
  exit(0);
}

/**
 * The work is done with a parent process part.
 */
static void execute_parent(pid_t pid, command_t* cmd) {
  if (cmd->flags & FLAG_BACKGROUND) {
    printf("yxsh: Running background: %d\n", (int) pid);
    return;
  }

  int status = 0;
  pid_t result = waitpid(pid, &status, WUNTRACED);
  if (result == -1)
    perror("Wait for child process termination error");
}

void execute(commandline_t* commandline, size_t ncmds) {
  for (size_t i = 0; i < ncmds; i++) {
    pid_t pid = fork();
    switch (pid) {
      case -1:
        perror("Process create error");
        return;
      case 0:
        execute_fork(&commandline->cmds[i]);
        return;
      default:
        execute_parent(pid, &commandline->cmds[i]);
    }
  }
}

