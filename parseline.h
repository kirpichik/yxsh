//
//  parseline.h
//  yxsh
//
//  Created by Кирилл on 02.04.2018.
//  Copyright © 2018 Кирилл. All rights reserved.
//

#ifndef _PARSELINE_H
#define _PARSELINE_H

#include "shell.h"

/**
 * Parse line to commands set.
 *
 * @param line Line for parse.
 * @param cmds Array of result commands.
 *
 * @return Commands count.
 */
int parseline(char* line, commandline_t* cmds);

/**
 * Free allocated memory for strings.
 *
 * @param cmds Array of result commands;
 * @param ncmds Commands count.
 */
void free_cmds_strings(commandline_t* cmds, size_t ncmds);

#endif
