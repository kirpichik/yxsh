//
//  shell.c
//  yxsh
//
//  Created by Кирилл on 14.03.2018.
//  Copyright © 2018 Кирилл. All rights reserved.
//

#include <stdio.h>

#include "shell.h"
#include "parseline.h"
#include "promptline.h"
#include "executor.h"

#define PROMPT "[yx!]> "

int main(int argc, char* argv[]) {
  commandline_t commandline;
  char line[1024];
  int ncmds;

  while (promptline(PROMPT, line, sizeof(line)) > 0) {
    if ((ncmds = parseline(line, &commandline)) <= 0)
      continue;
    execute(&commandline, ncmds);
  }
  return 0;
}

