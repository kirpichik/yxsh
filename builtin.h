//
//  builtin.h
//  yxsh
//
//  Created by Кирилл on 11.04.2018.
//  Copyright © 2018 Кирилл. All rights reserved.
//

#ifndef _BUILTIN_H
#define _BUILTIN_H

#include <stdbool.h>

#include "shell.h"

/**
 * Tries to execute the command as builtin function.
 *
 * @param cmd Command.
 *
 * @return true if success.
 */
bool try_builtin(command_t* cmd);

#endif

