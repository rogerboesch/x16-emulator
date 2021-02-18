
#ifndef platform_ios_h
#define platform_ios_h

#include <TargetConditionals.h> // Must also be checked later

#ifdef TARGET_OS_IPHONE

#include <stdio.h>

char* platform_get_base_path(void);
char* platform_get_gif_path(void);
double platform_get_ticks(void);

FILE* platform_file_open(char* path, char* mode);
size_t platform_file_read(FILE* fp, void* buf, size_t size, size_t mnemb);

void platform_render_buffer(uint8_t* framebuffer);

// Temporary, replace later

#define SDL_Keycode uint8_t
#define SDL_Scancode uint8_t
#define SDL_Color uint8_t

#define SDL_Surface void
#define SDL_Texture void
#define SDL_Window void

#define SDL_RWops FILE
#define SDL_RWread platform_file_read
#define SDL_RWseek fseek
#define SDL_RWwrite fwrite
#define SDL_RWFromFile platform_file_open
#define SDL_RWclose fclose
#define SDL_Renderer void
#define SDL_GameController void

#define SDL_GetTicks void
#define SDL_ReadU8
#define SDL_WriteU8

#endif
#endif /* platform_ios_h */
