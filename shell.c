//
//  shell.c
//  yxsh
//
//  Created by Кирилл on 14.03.2018.
//  Copyright © 2018 Кирилл. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "shell.h"
#include "parseline.h"
#include "promptline.h"
#include "executor.h"

#define PROMPT "[yx!]> "
#define INPUT_BUFF 1024

static void free_cmds_strings(commandline_t*, size_t);

int main(int argc, char* argv[]) {
  commandline_t commandline;
  char line[INPUT_BUFF];
  int ncmds;

  while (promptline(PROMPT, line, sizeof(line)) > 0) {
    if ((ncmds = parseline(line, &commandline)) > 0)
      execute(&commandline, ncmds);

    free_cmds_strings(&commandline, ncmds);
  }
  
  return 0;
}

static void free_cmds_strings(commandline_t* cmdline, size_t ncmds) {
  for (size_t i = 0; i < ncmds; i++) {
    if (cmdline->cmds[i].infile)
      free(cmdline->cmds[i].infile);
    if (cmdline->cmds[i].outfile)
      free(cmdline->cmds[i].outfile);

    size_t j = 0;
    while (cmdline->cmds[i].cmdargs[j])
      free(cmdline->cmds[i].cmdargs[j++]);
  }
}

