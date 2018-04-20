//
//  builtin.c
//  yxsh
//
//  Created by Kirill on 11.04.2018.
//  Copyright © 2018 Kirill. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtin.h"

#define PATH_SIZE 1024

static void change_dirrectory(command_t* cmd) {
  char path[PATH_SIZE];
  char* home;

  if (cmd->cmdargs[1] && cmd->cmdargs[2]) {
    fprintf(stderr, "cd: to many arguments");
    return;
  }

  if (strcpy(path, cmd->cmdargs[1]) == NULL) { // TODO - checks size
    perror("yxsh: cd");
    return;
  }

  if (!(home = getenv("HOME"))) {
    fprintf(stderr, "yxsh: Home path variable is not set.");
    return;
  }

  if (chdir(cmd->cmdargs[1])) {   
    perror("yxsh: cd");
  }
}

bool try_builtin(command_t* cmd) {
  if (!strcmp(cmd->cmdargs[0], "exit")) {
    // TODO - check background
    exit(0);
  } else if (!strcmp(cmd->cmdargs[0], "cd")) {
    change_dirrectory(cmd);
  } else
    return false;
  return true;
}

