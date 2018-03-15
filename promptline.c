//
//  parseline.cpp
//  yxsh
//
//  Created by Кирилл on 14.03.2018.
//  Copyright © 2018 Кирилл. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "promptline.h"

int promptline(char* prompt, char* line, size_t sizline) {
  int n = 0;

  write(1, prompt, strlen(prompt));
  while (1) {
    n += read(0, (line + n), sizline - n);
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
