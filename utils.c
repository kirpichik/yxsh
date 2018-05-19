//
//  utils.c
//  yxsh
//
//  Created by Kirill on 19.05.2018.
//  Copyright © 2018 Kirill. All rights reserved.
//

#include "utils.h"

char* str_realloc(char* str) {
  if (!str)
    return NULL;

  char* copy = (char*) malloc((strlen(str) + 1) * sizeof(char));
  if (!copy)
    return NULL;
  strcpy(copy, str);
  return copy;
}

bool cmddup(command_t* dest, command_t* cmd) {
  if (!cmd || !dest)
    return false;

  dest->flags = cmd->flags;
  dest->infile = str_realloc(cmd->infile);
  dest->outfile = str_realloc(cmd->outfile);

  size_t i = 0;
  while (cmd->cmdargs[i]) {
    dest->cmdargs[i] = str_realloc(cmd->cmdargs[i]);
    if (!(dest->cmdargs[i])) {
      perror("yxsh: Cannot allocate memory");
      while (i)
        free(dest->cmdargs[--i]);
      return false;
    }
    i++;
  }
  dest->cmdargs[i++] = NULL;
  dest->cmdargs[i] = NULL;
  return true;
}

