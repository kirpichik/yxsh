
#ifndef _PARSER_H
#define _PARSER_H

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

#endif

