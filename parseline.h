//
//  parseline.h
//  yxsh
//
//  Created by Кирилл on 14.03.2018.
//  Copyright © 2018 Кирилл. All rights reserved.
//

#ifndef _PARSELINE_H
#define _PARSELINE_H

#include <string>

#include "command.h"

/**
 * Parses user input line and returns ordered set of commands.
 *
 * @param input User input.
 *
 * @return Ordered set of commands.
 */
CommandsSet parseline(const std::string input);

#endif /* _PARSELINE_H */
