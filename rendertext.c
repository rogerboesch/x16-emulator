#include <string.h>
#ifdef __MINGW32__
#include <ctype.h>
#endif
#include "rendertext.h"
#include <ctype.h>
#include "platform-ios.h"

#define CHAR_WIDTH 5
#define CHAR_HEIGHT 7
#define TEXTURE_HEIGHT (CHAR_HEIGHT * 1)
#define TEXTURE_WIDTH (CHAR_WIDTH * 0x60)

// Text Area origin => debug area
int xPos = 0;
int yPos = 0;

// font texture
SDL_Texture *fontTexture;
int textureInitialized = 0;

// *******************************************************************************************
// left trim string
//
char *ltrim(char *s)
{
	while(isspace(*s)) s++;
	return s;
}

// *******************************************************************************************
//
//										Simple 5 x 7 Font
//
// *******************************************************************************************

static unsigned char fontdata[] = {
	0x00, 0x00, 0x00, 0x00, 0x00,   // 0x20 (space)
	0x00, 0x00, 0x5F, 0x00, 0x00,   // 0x21 '!'
	0x00, 0x07, 0x00, 0x07, 0x00,   // 0x22 '"'
	0x14, 0x7F, 0x14, 0x7F, 0x14,   // 0x23 '#'
	0x24, 0x2A, 0x7F, 0x2A, 0x12,   // 0x24 '$'
	0x23, 0x13, 0x08, 0x64, 0x62,   // 0x25 '%'
	0x36, 0x49, 0x55, 0x22, 0x50,   // 0x26 '&'
	0x00, 0x05, 0x03, 0x00, 0x00,   // 0x27 '''
	0x00, 0x1C, 0x22, 0x41, 0x00,   // 0x28 '('
	0x00, 0x41, 0x22, 0x1C, 0x00,   // 0x29 ')'
	0x08, 0x2A, 0x1C, 0x2A, 0x08,   // 0x2A '*'
	0x08, 0x08, 0x3E, 0x08, 0x08,   // 0x2B '+'
	0x00, 0x50, 0x30, 0x00, 0x00,   // 0x2C ','
	0x08, 0x08, 0x08, 0x08, 0x08,   // 0x2D '-'
	0x00, 0x60, 0x60, 0x00, 0x00,   // 0x2E '.'
	0x20, 0x10, 0x08, 0x04, 0x02,   // 0x2F '/'
	0x3E, 0x51, 0x49, 0x45, 0x3E,   // 0x30 '0'
	0x00, 0x42, 0x7F, 0x40, 0x00,   // 0x31 '1'
	0x42, 0x61, 0x51, 0x49, 0x46,   // 0x32 '2'
	0x21, 0x41, 0x45, 0x4B, 0x31,   // 0x33 '3'
	0x18, 0x14, 0x12, 0x7F, 0x10,   // 0x34 '4'
	0x27, 0x45, 0x45, 0x45, 0x39,   // 0x35 '5'
	0x3C, 0x4A, 0x49, 0x49, 0x30,   // 0x36 '6'
	0x01, 0x71, 0x09, 0x05, 0x03,   // 0x37 '7'
	0x36, 0x49, 0x49, 0x49, 0x36,   // 0x38 '8'
	0x06, 0x49, 0x49, 0x29, 0x1E,   // 0x39 '9'
	0x00, 0x36, 0x36, 0x00, 0x00,   // 0x3A ':'
	0x00, 0x56, 0x36, 0x00, 0x00,   // 0x3B ';'
	0x00, 0x08, 0x14, 0x22, 0x41,   // 0x3C '<'
	0x14, 0x14, 0x14, 0x14, 0x14,   // 0x3D '='
	0x41, 0x22, 0x14, 0x08, 0x00,   // 0x3E '>'
	0x02, 0x01, 0x51, 0x09, 0x06,   // 0x3F '?'
	0x32, 0x49, 0x79, 0x41, 0x3E,   // 0x40 '@'
	0x7E, 0x11, 0x11, 0x11, 0x7E,   // 0x41 'A'
	0x7F, 0x49, 0x49, 0x49, 0x36,   // 0x42 'B'
	0x3E, 0x41, 0x41, 0x41, 0x22,   // 0x43 'C'
	0x7F, 0x41, 0x41, 0x22, 0x1C,   // 0x44 'D'
	0x7F, 0x49, 0x49, 0x49, 0x41,   // 0x45 'E'
	0x7F, 0x09, 0x09, 0x01, 0x01,   // 0x46 'F'
	0x3E, 0x41, 0x41, 0x51, 0x32,   // 0x47 'G'
	0x7F, 0x08, 0x08, 0x08, 0x7F,   // 0x48 'H'
	0x00, 0x41, 0x7F, 0x41, 0x00,   // 0x49 'I'
	0x20, 0x40, 0x41, 0x3F, 0x01,   // 0x4A 'J'
	0x7F, 0x08, 0x14, 0x22, 0x41,   // 0x4B 'K'
	0x7F, 0x40, 0x40, 0x40, 0x40,   // 0x4C 'L'
	0x7F, 0x02, 0x04, 0x02, 0x7F,   // 0x4D 'M'
	0x7F, 0x04, 0x08, 0x10, 0x7F,   // 0x4E 'N'
	0x3E, 0x41, 0x41, 0x41, 0x3E,   // 0x4F 'O'
	0x7F, 0x09, 0x09, 0x09, 0x06,   // 0x50 'P'
	0x3E, 0x41, 0x51, 0x21, 0x5E,   // 0x51 'Q'
	0x7F, 0x09, 0x19, 0x29, 0x46,   // 0x52 'R'
	0x46, 0x49, 0x49, 0x49, 0x31,   // 0x53 'S'
	0x01, 0x01, 0x7F, 0x01, 0x01,   // 0x54 'T'
	0x3F, 0x40, 0x40, 0x40, 0x3F,   // 0x55 'U'
	0x1F, 0x20, 0x40, 0x20, 0x1F,   // 0x56 'V'
	0x7F, 0x20, 0x18, 0x20, 0x7F,   // 0x57 'W'
	0x63, 0x14, 0x08, 0x14, 0x63,   // 0x58 'X'
	0x03, 0x04, 0x78, 0x04, 0x03,   // 0x59 'Y'
	0x61, 0x51, 0x49, 0x45, 0x43,   // 0x5A 'Z'
	0x00, 0x00, 0x7F, 0x41, 0x41,   // 0x5B '['
	0x02, 0x04, 0x08, 0x10, 0x20,   // 0x5C '\'
	0x41, 0x41, 0x7F, 0x00, 0x00,   // 0x5D ']'
	0x04, 0x02, 0x01, 0x02, 0x04,   // 0x5E '^'
	0x40, 0x40, 0x40, 0x40, 0x40,   // 0x5F '_'
	0x00, 0x01, 0x02, 0x04, 0x00,   // 0x60 '`'
	0x20, 0x54, 0x54, 0x54, 0x78,   // 0x61 'a'
	0x7F, 0x48, 0x44, 0x44, 0x38,   // 0x62 'b'
	0x38, 0x44, 0x44, 0x44, 0x20,   // 0x63 'c'
	0x38, 0x44, 0x44, 0x48, 0x7F,   // 0x64 'd'
	0x38, 0x54, 0x54, 0x54, 0x18,   // 0x65 'e'
	0x08, 0x7E, 0x09, 0x01, 0x02,   // 0x66 'f'
	0x08, 0x14, 0x54, 0x54, 0x3C,   // 0x67 'g'
	0x7F, 0x08, 0x04, 0x04, 0x78,   // 0x68 'h'
	0x00, 0x44, 0x7D, 0x40, 0x00,   // 0x69 'i'
	0x20, 0x40, 0x44, 0x3D, 0x00,   // 0x6A 'j'
	0x00, 0x7F, 0x10, 0x28, 0x44,   // 0x6B 'k'
	0x00, 0x41, 0x7F, 0x40, 0x00,   // 0x6C 'l'
	0x7C, 0x04, 0x18, 0x04, 0x78,   // 0x6D 'm'
	0x7C, 0x08, 0x04, 0x04, 0x78,   // 0x6E 'n'
	0x38, 0x44, 0x44, 0x44, 0x38,   // 0x6F 'o'
	0x7C, 0x14, 0x14, 0x14, 0x08,   // 0x70 'p'
	0x08, 0x14, 0x14, 0x18, 0x7C,   // 0x71 'q'
	0x7C, 0x08, 0x04, 0x04, 0x08,   // 0x72 'r'
	0x48, 0x54, 0x54, 0x54, 0x20,   // 0x73 's'
	0x04, 0x3F, 0x44, 0x40, 0x20,   // 0x74 't'
	0x3C, 0x40, 0x40, 0x20, 0x7C,   // 0x75 'u'
	0x1C, 0x20, 0x40, 0x20, 0x1C,   // 0x76 'v'
	0x3C, 0x40, 0x30, 0x40, 0x3C,   // 0x77 'w'
	0x44, 0x28, 0x10, 0x28, 0x44,   // 0x78 'x'
	0x0C, 0x50, 0x50, 0x50, 0x3C,   // 0x79 'y'
	0x44, 0x64, 0x54, 0x4C, 0x44,   // 0x7A 'z'
	0x00, 0x08, 0x36, 0x41, 0x00,   // 0x7B '{'
	0x00, 0x00, 0x7F, 0x00, 0x00,   // 0x7C '|'
	0x00, 0x41, 0x36, 0x08, 0x00,   // 0x7D '}'
	0x08, 0x08, 0x2A, 0x1C, 0x08,   // 0x7E ->'
	0x08, 0x1C, 0x2A, 0x08, 0x08,   // 0x7F <-'
};

// *******************************************************************************************
//
//										Initialize charset
//
// *******************************************************************************************

void DEBUGInitChars(SDL_Renderer *renderer) {
    // TODO: Implement
/*
    uint16_t textureData[TEXTURE_WIDTH * TEXTURE_HEIGHT];
	memset(textureData, 0, sizeof textureData);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
	fontTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA4444, SDL_TEXTUREACCESS_STATIC, TEXTURE_WIDTH, TEXTURE_HEIGHT);
	for (int ch =0x00; ch<0x60; ch++){
		int rcx = 0;
		for (int x1 = 0; x1 < 5; x1++) {
			int rcy = 0;
			int pixData = fontdata[ch * 5 + x1];
			while (pixData != 0) {
				textureData[ch*CHAR_WIDTH + rcy*TEXTURE_WIDTH + rcx] = (pixData & 1) ? 0xFFFF : 0x0000;
				pixData = pixData >> 1;
				rcy++;
			}
			rcx++;
		}
	}
	SDL_UpdateTexture(fontTexture, NULL, &textureData, TEXTURE_WIDTH*2);
	textureInitialized = 1;
 */
}

// *******************************************************************************************
//
//										Write character
//
// *******************************************************************************************

void DEBUGWrite(SDL_Renderer *renderer, int x, int y, int ch, SDL_Color colour) {
    // TODO: Implement

    /*
	if (!textureInitialized) {
		DEBUGInitChars(renderer);
	}
	SDL_SetTextureColorMod(fontTexture, colour.r, colour.g, colour.b);
	ch-=0x20;
	SDL_Rect srcRect = {
		ch* CHAR_WIDTH,
		0,
		CHAR_WIDTH,
		CHAR_HEIGHT
	};
	SDL_Rect dstRect = {
		x*(CHAR_WIDTH+1) + xPos,
		y*(CHAR_HEIGHT+1) + yPos,
		CHAR_WIDTH,
		CHAR_HEIGHT
	};
	SDL_RenderCopy(renderer, fontTexture, &srcRect, &dstRect);
     */
}

// *******************************************************************************************
//
//										Write String
//
// *******************************************************************************************

void DEBUGString(SDL_Renderer *renderer, int x, int y, char *s, SDL_Color colour) {
	while (*s != '\0') {
		DEBUGWrite(renderer, x++, y, *s++, colour);
	}
}
