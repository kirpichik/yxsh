//
//  shell.c
//  yxsh
//
//  Created by Kirill on 14.03.2018.
//  Copyright © 2018 Kirill. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "executor.h"
#include "parseline.h"
#include "promptline.h"
#include "shell.h"

#define PROMPT "[yx!]> "
#define INPUT_BUFF 1024

int main(int argc, char* argv[]) {
  commandline_t commandline;
  char line[INPUT_BUFF];
  int ncmds;

  while (promptline(PROMPT, line, sizeof(line)) > 0) {
    if ((ncmds = parseline(line, &commandline)) > 0) {
      execute(&commandline, ncmds);
      free_cmds_strings(&commandline, ncmds);
    }
  }

  return 0;
}

