//
//  promptline.cpp
//  yxsh
//
//  Created by Кирилл on 14.03.2018.
//  Copyright © 2018 Кирилл. All rights reserved.
//

#include <string>
#include <unistd.h>

#include "promptline.h"

const std::string INVITE_STRING = ">> ";

ssize_t promptline(char* buffer, size_t max) {
  if (max == 0)
    return -1;
  
  ssize_t len = write(STDOUT_FILENO, INVITE_STRING.c_str(), INVITE_STRING.size());
  
  if (len == -1)
    return -1;
  
  if ((len = read(STDIN_FILENO, buffer, max - 1)) != -1)
    buffer[len] = '\0';
  
  return len;
}
