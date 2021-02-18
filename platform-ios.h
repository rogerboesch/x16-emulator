
#ifndef platform_ios_h
#define platform_ios_h

#include <TargetConditionals.h> // Must also be checked later

#ifdef TARGET_OS_IPHONE

// Temporary, replace later

#define SDL_Keycode uint8_t
#define SDL_Scancode uint8_t
#define SDL_Color uint8_t

#define SDL_Surface void
#define SDL_Texture void
#define SDL_Window void

#define SDL_RWops FILE
#define SDL_RWread fread
#define SDL_RWseek fseek
#define SDL_RWwrite fwrite
#define SDL_RWFromFile fopen
#define SDL_RWclose fclose
#define SDL_Renderer void
#define SDL_GameController void

#define SDL_GetTicks void
#define SDL_ReadU8
#define SDL_WriteU8

char* platform_get_base_path(void);
int platform_get_ticks(void);

#endif
#endif /* platform_ios_h */
