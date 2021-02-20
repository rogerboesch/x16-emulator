// Commander X16 Emulator
// Copyright (c) 2019 Michael Steil
// All rights reserved. License: 2-clause BSD

#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "glue.h"
#include "platform_ios.h"

#define ADDR_VRAM_START     0x00000
#define ADDR_VRAM_END       0x20000
#define ADDR_PSG_START      0x1F9C0
#define ADDR_PSG_END        0x1FA00
#define ADDR_PALETTE_START  0x1FA00
#define ADDR_PALETTE_END    0x1FC00
#define ADDR_SPRDATA_START  0x1FC00
#define ADDR_SPRDATA_END    0x20000

#define NUM_SPRITES 128

// both VGA and NTSC
#define SCAN_WIDTH 800
#define SCAN_HEIGHT 525

// VGA
#define VGA_FRONT_PORCH_X 16
#define VGA_FRONT_PORCH_Y 10
#define VGA_PIXEL_FREQ 25.175

// NTSC: 262.5 lines per frame, lower field first
#define NTSC_FRONT_PORCH_X 80
#define NTSC_FRONT_PORCH_Y 22
#define NTSC_PIXEL_FREQ (15.750 * 800 / 1000)
#define TITLE_SAFE_X 0.067
#define TITLE_SAFE_Y 0.05

// visible area we're drawing
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define SCREEN_RAM_OFFSET 0x00000

// When rendering a layer line, we can amortize some of the cost by calculating multiple pixels at a time.
#define LAYER_PIXELS_PER_ITERATION 8

bool video_init(int window_scale, char *quality);
void video_reset(void);
bool video_step(float mhz);
bool video_update(void);
void video_end(void);
bool video_get_irq_out(void);
void video_save(SDL_RWops *f);
uint8_t video_read(uint8_t reg, bool debugOn);
void video_write(uint8_t reg, uint8_t value);
void video_update_title(const char* window_title);

uint8_t via1_read(uint8_t reg);
void via1_write(uint8_t reg, uint8_t value);

// For debugging purposes only:
uint8_t video_space_read(uint32_t address);
void video_space_write(uint32_t address, uint8_t value);

bool video_is_tilemap_address(int addr);
bool video_is_tiledata_address(int addr);
bool video_is_special_address(int addr);

#endif
