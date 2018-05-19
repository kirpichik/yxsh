//
//  utils.h
//  yxsh
//
//  Created by Kirill on 19.05.2018.
//  Copyright © 2018 Kirill. All rights reserved.
//

#ifndef _UTILS_H
#define _UTILS_H

/**
 * Creates copy of string at heap.
 *
 * @param str String to copy.
 *
 * @return Copied string.
 */
char* str_realloc(char* str);

/**
 * Duplicate the command.
 *
 * @param dest Destination command.
 * @param cmd Command to duplicate
 */
bool cmddup(command_t* dest, command_t* cmd);

#endif

