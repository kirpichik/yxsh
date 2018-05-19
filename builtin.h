//
//  builtin.h
//  yxsh
//
//  Created by Kirill on 11.04.2018.
//  Copyright © 2018 Kirill. All rights reserved.
//

#ifndef _BUILTIN_H
#define _BUILTIN_H

#include <stdbool.h>

#include "tasks.h"
#include "shell.h"

#define BUILTIN_EXECUTED 1
#define BUILTIN_NOTFOUND 2
#define BUILTIN_EXIT     3

/**
 * Tries to execute the command as builtin function.
 *
 * @param env Environment.
 * @param cmd Command.
 *
 * @return Result of try.
 */
int try_builtin(tasks_env_t* env, command_t* cmd);

#endif /* _BUILTIN_H */

