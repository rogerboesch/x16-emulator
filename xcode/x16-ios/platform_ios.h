//
//  platform_ios.h
//  Platform specfic includes for iOS (non SDL)
//
//  Written 2021 by Roger Boesch
//  "You can do whatever you like with it"
//

#ifndef platform_ios_h
#define platform_ios_h

#include <TargetConditionals.h> // Must also be checked later

#ifdef TARGET_OS_IPHONE

#define APP_STORE 1

#include <stdio.h>

// Types (temporary)
typedef struct platform_color {
    uint8_t r,g,b,a;
} PLATFORM_COLOR;

typedef struct platform_rect {
    int x,y,w,h;
} PLATFORM_RECT;

// Path support
char* platform_get_base_path(void);
char* platform_get_gif_path(void);
char* platform_get_documents_path(void);
char* platform_get_sdcard_path(char* name);
double platform_get_ticks(void);

// File support
FILE* platform_file_open(char* path, char* mode);
void platform_file_close(FILE* fp);
size_t platform_file_read(FILE* fp, void* buf, size_t size, size_t mnemb);
uint8_t platform_file_read_u8(FILE* fp);
size_t platform_file_write(FILE* fp, void* buf, size_t size, size_t mnemb);
void platform_file_write_u8(FILE* fp, uint8_t byte);
int platform_file_seek(FILE *stream, long int offset, int whence);

// Cloud support
#ifndef APP_STORE
void platform_load_external_file(void);
void platform_load_external_file_async(void); // Change later
char* platform_wait_for_external_filename(void);
#endif

// Render support
void platform_render_buffer(uint8_t* framebuffer);

// Paste support
char* platform_get_from_clipboard(void);

// Temporary, will be replaced later
#define SDL_Keycode uint8_t
#define SDL_Scancode uint8_t
#define SDL_Rect PLATFORM_RECT
#define SDL_Color PLATFORM_COLOR

// Replace this later by platform independent types
#define SDL_Renderer void
#define SDL_GameController void

// Replace this later by directly call platform_xx functions
#define SDL_RWops FILE
#define SDL_RWread platform_file_read
#define SDL_RWseek platform_file_seek
#define SDL_RWwrite platform_file_write
#define SDL_RWFromFile platform_file_open
#define SDL_RWclose platform_file_close
#define SDL_ReadU8 platform_file_read_u8
#define SDL_WriteU8 platform_file_write_u8

#endif
#endif /* platform_ios_h */
