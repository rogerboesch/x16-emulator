// Commander X16 Emulator
// Copyright (c) 2019 Michael Steil
// All rights reserved. License: 2-clause BSD
#ifndef _SD_CARD_H_
#define _SD_CARD_H_
#include <inttypes.h>
#include <stdbool.h>
#include "platform_ios.h"

extern FILE *sdcard_file;
extern bool sdcard_attached;

void sdcard_attach(void);
void sdcard_detach(void);

void sdcard_select(bool select);
uint8_t sdcard_handle(uint8_t inbyte);

#endif
