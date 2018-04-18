//
//  executor.h
//  yxsh
//
//  Created by Кирилл on 02.04.2018.
//  Copyright © 2018 Кирилл. All rights reserved.
//

#ifndef _EXECUTOR_H
#define _EXECUTOR_H

#include "shell.h"

/**
 * Executes commandline.
 *
 * @param commandline Commandline for execution.
 * @param ncmds Number of commands in commandline.
 */
void execute(commandline_t* commandline, size_t ncmds);

#endif

