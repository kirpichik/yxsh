//
//  parseline.cpp
//  yxsh
//
//  Created by Кирилл on 14.03.2018.
//  Copyright © 2018 Кирилл. All rights reserved.
//

#include <sstream>

#include "parseline.h"

/**
 * Type of readed token.
 */
enum TokenType {
  
};

CommandsSet parseline(const std::string input) {
  std::stringstream stream(input);
  
  std::string word;
  while(stream >> word) {
    
  }
  
  return CommandsSet();
}
