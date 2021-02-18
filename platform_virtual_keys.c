//
//  rb_virtual_keys.c
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#include <string.h>
#include "platform_virtual_keys.h"

int character_to_vk(char ch) {
    const char* position_ptr = strchr(KeyCharacterSet, ch);
    long position = (position_ptr == NULL ? RBVK_Unknown : position_ptr - KeyCharacterSet);
    return (int)position;
}
