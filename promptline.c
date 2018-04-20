//
//  promptline.c
//  yxsh
//
//  Created by Kirill on 14.03.2018.
//  Copyright © 2018 Kirill. All rights reserved.
//

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "promptline.h"

int promptline(char* line, size_t buff_len) {
  int n = 0;

  while (1) {
    n += read(STDIN_FILENO, (line + n), buff_len - n);
    *(line + n) = '\0';
    /*
     *  check to see if command line extends onto
     *  next line.  If so, append next line to command line
     */

    if (*(line + n - 2) == '\\' && *(line + n - 1) == '\n') {
      *(line + n) = ' ';
      *(line + n - 1) = ' ';
      *(line + n - 2) = ' ';
      continue; /*  read next line  */
    }
    return (n); /* all done */
  }
}

