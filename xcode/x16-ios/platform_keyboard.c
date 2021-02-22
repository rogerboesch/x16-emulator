//
//  platform_keyboard.c
//  Replaces keyboard.c from original Commander X16 emulator
//
//  Written 2021 by Roger Boesch
//  "You can do whatever you like with it"
//

#include <stdio.h>
#include <stdbool.h>
#include "glue.h"
#include "ps2.h"
#include "keyboard.h"
#include "platform_virtual_keys.h"

#define EXTENDED_FLAG 0x100
#define ESC_IS_BREAK /* if enabled, Esc sends Break/Pause key instead of Esc */

int ps2_scancode_from_virtual_code(RBVirtualKey scancode) {
    switch (scancode) {
        case RBVK_Grave:
            return 0x0e;
        case RBVK_BackSpace:
            return 0x66;
        case RBVK_Tab:
            return 0xd;
        case RBVK_Return:
            return 0x5a;
        case RBVK_Escape:
#ifdef ESC_IS_BREAK
            return 0xff;
#else
            return 0x76;
#endif
        case RBVK_Space:
            return 0x29;
        case RBVK_Quote: // TODO: Check later
            return 0x52;
        case RBVK_Comma:
            return 0x41;
        case RBVK_Minus:
            return 0x4e;
        case RBVK_Period:
            return 0x49;
        case RBVK_Slash:
            return 0x4a;
        case RBVK_Num0:
            return 0x45;
        case RBVK_Num1:
            return 0x16;
        case RBVK_Num2:
            return 0x1e;
        case RBVK_Num3:
            return 0x26;
        case RBVK_Num4:
            return 0x25;
        case RBVK_Num5:
            return 0x2e;
        case RBVK_Num6:
            return 0x36;
        case RBVK_Num7:
            return 0x3d;
        case RBVK_Num8:
            return 0x3e;
        case RBVK_Num9:
            return 0x46;
        case RBVK_SemiColon:
            return 0x4c;
        case RBVK_Equal:
            return 0x55;
        case RBVK_LBracket:
            return 0x54;
        case RBVK_BackSlash:
            return 0x5d;
        case RBVK_RBracket:
            return 0x5b;
        case RBVK_A:
            return 0x1c;
        case RBVK_B:
            return 0x32;
        case RBVK_C:
            return 0x21;
        case RBVK_D:
            return 0x23;
        case RBVK_E:
            return 0x24;
        case RBVK_F:
            return 0x2b;
        case RBVK_G:
            return 0x34;
        case RBVK_H:
            return 0x33;
        case RBVK_I:
            return 0x43;
        case RBVK_J:
            return 0x3B;
        case RBVK_K:
            return 0x42;
        case RBVK_L:
            return 0x4B;
        case RBVK_M:
            return 0x3A;
        case RBVK_N:
            return 0x31;
        case RBVK_O:
            return 0x44;
        case RBVK_P:
            return 0x4D;
        case RBVK_Q:
            return 0x15;
        case RBVK_R:
            return 0x2D;
        case RBVK_S:
            return 0x1B;
        case RBVK_T:
            return 0x2C;
        case RBVK_U:
            return 0x3C;
        case RBVK_V:
            return 0x2A;
        case RBVK_W:
            return 0x1D;
        case RBVK_X:
            return 0x22;
        case RBVK_Y:
            return 0x35;
        case RBVK_Z:
            return 0x1A;
        case RBVK_Delete:
            return 0;
        case RBVK_Up:
            return 0x75 | EXTENDED_FLAG;
        case RBVK_Down:
            return 0x72 | EXTENDED_FLAG;
        case RBVK_Right:
            return 0x74 | EXTENDED_FLAG;
        case RBVK_Left:
            return 0x6b | EXTENDED_FLAG;
        case RBVK_Insert:
            return 0;
        case RBVK_Home:
            return 0x6c | EXTENDED_FLAG;
        case RBVK_End:
            return 0;
        case RBVK_PageUp:
            return 0;
        case RBVK_PageDown:
            return 0;
        case RBVK_F1:
            return 0x05;
        case RBVK_F2:
            return 0x06;
        case RBVK_F3:
            return 0x04;
        case RBVK_F4:
            return 0x0c;
        case RBVK_F5:
            return 0x03;
        case RBVK_F6:
            return 0x0b;
        case RBVK_F7:
            return 0x83;
        case RBVK_F8:
            return 0x0a;
        case RBVK_F9:
            return 0x01;
        case RBVK_F10:
            return 0x09;
        case RBVK_F11:
            return 0x78;
        case RBVK_F12:
            return 0x07;
        case RBVK_RShift:
            return 0x59;
        case RBVK_LShift:
            return 0x12;
        case RBVK_LControl:
            return 0x14;
        case RBVK_RControl:
            return 0x14 | EXTENDED_FLAG;
        case RBVK_LAlt:
            return 0x11;
        case RBVK_RAlt:
            return 0x11 | EXTENDED_FLAG;
        case RBVK_Numpad0:
            return 0x70;
        case RBVK_Numpad1:
            return 0x69;
        case RBVK_Numpad2:
            return 0x72;
        case RBVK_Numpad3:
            return 0x7a;
        case RBVK_Numpad4:
            return 0x6b;
        case RBVK_Numpad5:
            return 0x73;
        case RBVK_Numpad6:
            return 0x74;
        case RBVK_Numpad7:
            return 0x6c;
        case RBVK_Numpad8:
            return 0x75;
        case RBVK_Numpad9:
            return 0x7d;
        case RBVK_Add:
            return 0x79;
        case RBVK_Multiply:
            return 0x7c;
        case RBVK_Divide:
            return 0x4a | EXTENDED_FLAG;

//      case SDL_SCANCODE_KP_PERIOD:
//          return 0x71;
//      case SDL_SCANCODE_KP_MINUS:
//          return 0x7b;
//      case SDL_SCANCODE_LGUI: // Windows/Command
//          return 0x5b | EXTENDED_FLAG;
//      case SDL_SCANCODE_NONUSBACKSLASH:
//          return 0x61;
//      case SDL_SCANCODE_KP_ENTER:
//          return 0x5a | EXTENDED_FLAG;
//      case SDL_SCANCODE_CLEAR:
//          return 0;
//      case SDL_SCANCODE_PAUSE:
//          return 0;

        default:
            return 0;
    }
     
    return 0;
}

void handle_keyboard(bool down, SDL_Keycode sym, SDL_Scancode scancode) {
    if (down) {
        if (log_keyboard) {
            printf("DOWN 0x%02X\n", scancode);
            fflush(stdout);
        }

        int ps2_scancode = ps2_scancode_from_virtual_code(scancode);
        
        if (ps2_scancode == 0xff) {
            // "Pause/Break" sequence
            ps2_buffer_add(0, 0xe1);
            ps2_buffer_add(0, 0x14);
            ps2_buffer_add(0, 0x77);
            ps2_buffer_add(0, 0xe1);
            ps2_buffer_add(0, 0xf0);
            ps2_buffer_add(0, 0x14);
            ps2_buffer_add(0, 0xf0);
            ps2_buffer_add(0, 0x77);
        }
        else {
            if (ps2_scancode & EXTENDED_FLAG) {
                ps2_buffer_add(0, 0xe0);
            }
            
            ps2_buffer_add(0, ps2_scancode & 0xff);
        }
    }
    else {
        if (log_keyboard) {
            printf("UP   0x%02X\n", scancode);
            fflush(stdout);
        }

        int ps2_scancode = ps2_scancode_from_virtual_code(scancode);
        if (ps2_scancode & EXTENDED_FLAG) {
            ps2_buffer_add(0, 0xe0);
        }
        
        ps2_buffer_add(0, 0xf0); // BREAK
        ps2_buffer_add(0, ps2_scancode & 0xff);
    }
}
