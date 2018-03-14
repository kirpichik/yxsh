//
//  promptline.h
//  yxsh
//
//  Created by Кирилл on 14.03.2018.
//  Copyright © 2018 Кирилл. All rights reserved.
//

#ifndef _PROMTPLINE_H
#define _PROMTPLINE_H

#include <stddef.h>

/**
 * Reads user input from stdin and store it to the buffer.
 *
 * @param buffer Store buffer.
 * @param max Buffer size.
 *
 * @return -1, if read error or readed length.
 */
ssize_t promptline(char* buffer, size_t max);

#endif /* _PROMTPLINE_H */
