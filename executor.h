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
 * @param ncmds Number of commands in commandline.
 */
void execute(tasks_env_t* env, commandline_t* commandline, size_t ncmds);

#endif /* _EXECUTOR_H */
