//
//  executor.h
//  yxsh
//
//  Created by Kirill on 02.04.2018.
//  Copyright © 2018 Kirill. All rights reserved.
//

#ifndef _EXECUTOR_H
#define _EXECUTOR_H

#include "shell.h"
#include "tasks.h"

/**
 * Executes commandline.
 *
 * @param env Current tasks environment.
 * @param commandline Commandline for execution.
 *
 * @return true if program can be continued.
 */
bool execute(tasks_env_t* env, commandline_t* commandline);

#endif /* _EXECUTOR_H */

