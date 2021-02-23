//
//  platform_debugger.c
//  Debugger support on iOS (not yet working)
//
//  Replaces debugger.c from original Commander X16 emulator written by Paul Robson
//
//  Written 2021 by Roger Boesch
//  "You can do whatever you like with it"
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "glue.h"
#include "disasm.h"
#include "memory.h"
#include "video.h"
#include "../../cpu/fake6502.h"
#include "debugger.h"
#include "rendertext.h"
#include "platform_ios.h"
#include "platform_virtual_keys.h"

extern int poll_events(RBEvent* event);

static void DEBUGHandleKeyEvent(RBVirtualKey key,int isShift);

static void DEBUGNumber(int x,int y,int n,int w, SDL_Color colour);
static void DEBUGAddress(int x, int y, int bank, int addr, SDL_Color colour);
static void DEBUGVAddress(int x, int y, int addr, SDL_Color colour);

static void DEBUGRenderData(int y,int data);
static void DEBUGRenderZeroPageRegisters(int y);
static int DEBUGRenderRegisters(void);
static void DEBUGRenderVRAM(int y, int data);
static void DEBUGRenderCode(int lines,int initialPC);
static void DEBUGRenderStack(int bytesCount);
static void DEBUGRenderCmdLine(int x, int width, int height);
static bool DEBUGBuildCmdLine(RBVirtualKey key);
static void DEBUGExecCmd(void);

// *******************************************************************************************
//
//        This is the minimum-interference flag. It's designed so that when
//        its non-zero DEBUGRenderDisplay() is called.
//
//            if (showDebugOnRender != 0) {
//                DEBUGRenderDisplay(SCREEN_WIDTH,SCREEN_HEIGHT,renderer);
//                SDL_RenderPresent(renderer);
//                return true;
//            }
//
//        before the SDL_RenderPresent call in video_update() in video.c
//
//        This controls what is happening. It is at the top of the main loop in main.c
//
//            if (isDebuggerEnabled != 0) {
//                int dbgCmd = DEBUGGetCurrentStatus();
//                if (dbgCmd > 0) continue;
//                if (dbgCmd < 0) break;
//            }
//
//        Both video.c and main.c require debugger.h to be included.
//
//        isDebuggerEnabled should be a flag set as a command line switch - without it
//        it will run unchanged. It should not be necessary to test the render code
//        because showDebugOnRender is statically initialised to zero and will only
//        change if DEBUGGetCurrentStatus() is called.
//
// *******************************************************************************************

//
// 0-9A-F sets the program address, with shift sets the data address.
//
#define DBGKEY_HOME         RBVK_F1                          // F1 is "Goto PC"
#define DBGKEY_RESET        RBVK_F2                          // F2 resets the 6502
#define DBGKEY_RUN          RBVK_F5                          // F5 is run.
#define DBGKEY_SETBRK       RBVK_F9                          // F9 sets breakpoint
#define DBGKEY_STEP         RBVK_F11                         // F11 is step into.
#define DBGKEY_STEPOVER     RBVK_F10                         // F10 is step over.
#define DBGKEY_PAGE_NEXT    RBVK_Add
#define DBGKEY_PAGE_PREV    RBVK_Minus

#define DBGSCANKEY_BRK      RBVK_F12                         // F12 is break into running code.
#define DBGSCANKEY_SHOW     RBVK_Tab                         // Show screen key.

#define DBGMAX_ZERO_PAGE_REGISTERS 20

#define DDUMP_RAM    0
#define DDUMP_VERA    1

enum DBG_CMD { CMD_DUMP_MEM='m', CMD_DUMP_VERA='v', CMD_DISASM='d', CMD_SET_BANK='b', CMD_SET_REGISTER='r', CMD_FILL_MEMORY='f' };

// RGB colours
const SDL_Color col_bkgnd= {0, 0, 0, 255};
const SDL_Color col_label= {0, 255, 0, 255};
const SDL_Color col_data= {0, 255, 255, 255};
const SDL_Color col_highlight= {255, 255, 0, 255};
const SDL_Color col_cmdLine= {255, 255, 255, 255};

const SDL_Color col_vram_tilemap = {0, 255, 255, 255};
const SDL_Color col_vram_tiledata = {0, 255, 0, 255};
const SDL_Color col_vram_special  = {255, 92, 92, 255};
const SDL_Color col_vram_other  = {128, 128, 128, 255};

int showDebugOnRender = 0;                                      // Used to trigger rendering in video.c
int showFullDisplay = 0;                                        // If non-zero show the whole thing.
int currentPC = -1;                                             // Current PC value.
int currentData = 0;                                            // Current data display address.
int currentPCBank = -1;
int currentBank = -1;
int currentMode = DMODE_RUN;                                    // Start running.
int breakPoint = -1;                                            // User Break
int stepBreakPoint = -1;                                        // Single step break.
int dumpmode          = DDUMP_RAM;

char cmdLine[64]= "";                                            // command line buffer
int currentPosInLine= 0;                                         // cursor position in the buffer (NOT USED _YET_)
int currentLineLen= 0;                                           // command line buffer length

int oldRegisters[DBGMAX_ZERO_PAGE_REGISTERS];                    // Old ZP Register values, for change detection
char* oldRegChange[DBGMAX_ZERO_PAGE_REGISTERS];                  // Change notification flags for output
int oldRegisterTicks = 0;                                        // Last PC when change notification was run

// *******************************************************************************************
//
//            This is used to determine who is in control. If it returns zero then
//            everything runs normally till the next call.
//            If it returns +ve, then it will update the video, and loop round.
//            If it returns -ve, then exit.
//
// *******************************************************************************************

int  DEBUGGetCurrentStatus(void) {
    RBEvent event;
    
    if (currentPC < 0) currentPC = pc;                            // Initialise current PC displayed.

    if (currentMode == DMODE_STEP) {                              // Single step before
        currentPC = pc;                                           // Update current PC
        currentMode = DMODE_STOP;                                 // So now stop, as we've done it.
    }

    if (pc == breakPoint || pc == stepBreakPoint) {                // Hit a breakpoint.
        currentPC = pc;                                            // Update current PC
        currentMode = DMODE_STOP;                                  // So now stop, as we've done it.
        stepBreakPoint = -1;                                       // Clear step breakpoint.
    }

    if (currentPCBank<0 && currentPC >= 0xA000) {
        currentPCBank= currentPC < 0xC000 ? memory_get_ram_bank() : memory_get_rom_bank();
    }

    if (currentMode != DMODE_RUN) {
        while (poll_events(&event)) {
            switch (event.type) {
                case RBEVT_Quit:
                    return -1;

                case RBEVT_KeyPressed:
                    switch (event.code) {
                        case DBGSCANKEY_BRK:
                            currentMode = DMODE_STOP;
                            currentPC = pc;
                            break;
                        case DBGSCANKEY_SHOW:
                            showFullDisplay = true;
                            break;
                        default:
                            DEBUGHandleKeyEvent(event.code, event.shift);
                            break;

                    }

                default:
                    break;
            }
        }
    }

    showDebugOnRender = (currentMode != DMODE_RUN);
    if (currentMode == DMODE_STOP) {
        video_update();
        return 1;
    }

    return 0;
}

void DEBUGInitUI(SDL_Renderer *pRenderer) {
    DEBUGInitChars(pRenderer);
}

void DEBUGFreeUI() {}

void DEBUGSetBreakPoint(int newBreakPoint) {
    breakPoint = newBreakPoint;
}

void DEBUGBreakToDebugger(void) {
    currentMode = DMODE_STOP;
    currentPC = pc;
}

static void DEBUGHandleKeyEvent(RBVirtualKey key, int isShift) {
    int opcode;

    switch(key) {

        case DBGKEY_STEP:
            currentMode = DMODE_STEP;
            break;

        case DBGKEY_STEPOVER:
            opcode = real_read6502(pc, false, 0);
            
            if (opcode == 0x20) {
                stepBreakPoint = pc + 3;
                currentMode = DMODE_RUN;
            }
            else {
                currentMode = DMODE_STEP;
            }
            break;

        case DBGKEY_RUN:
            currentMode = DMODE_RUN;
            break;

        case DBGKEY_SETBRK:
            breakPoint = currentPC;
            break;

        case DBGKEY_HOME:
            currentPC = pc;
            currentPCBank= currentPC < 0xC000 ? memory_get_ram_bank() : memory_get_rom_bank();
            break;

        case DBGKEY_RESET:
            reset6502();
            currentPC = pc;
            currentPCBank= -1;
            break;

        case DBGKEY_PAGE_NEXT:
            currentBank += 1;
            break;

        case DBGKEY_PAGE_PREV:
            currentBank -= 1;
            break;

        case RBVK_PageDown:
            if (dumpmode == DDUMP_RAM) {
                currentData = (currentData + 0x128) & 0xFFFF;
            }
            else {
                currentData = (currentData + 0x250) & 0x1FFFF;
            }
            break;

        case RBVK_PageUp:
            if (dumpmode == DDUMP_RAM) {
                currentData = (currentData - 0x128) & 0xFFFF;
            }
            else {
                currentData = (currentData - 0x250) & 0x1FFFF;
            }
            break;

        default:
            if(DEBUGBuildCmdLine(key)) {
                DEBUGExecCmd();
            }
            break;
    }
}

char kNUM_KEYPAD_CHARS[10] = {'1','2','3','4','5','6','7','8','9','0'};

static bool DEBUGBuildCmdLine(RBVirtualKey key) {
    // TODO: Fix input with correct chars
    if (currentLineLen <= sizeof(cmdLine)) {
        if ((key >= RBVK_Space && key <= RBVK_At) ||
            (key >= RBVK_LBracket && key <= RBVK_Z) ||
            (key >= RBVK_Num1 && key <= RBVK_Num0)) {

            cmdLine[currentPosInLine++] = (key >= RBVK_Num1) ? kNUM_KEYPAD_CHARS[key-RBVK_Num1] : key;
            
            if (currentPosInLine > currentLineLen) {
                currentLineLen++;
            }
        }
        else if (key == RBVK_BackSpace) {
            currentPosInLine--;
            
            if (currentPosInLine < 0) {
                currentPosInLine = 0;
            }
            
            currentLineLen--;
            
            if(currentLineLen < 0) {
                currentLineLen = 0;
            }
        }
        
        cmdLine[currentLineLen] = 0;
    }

    return (key == RBVK_Return) ;
}

static void DEBUGExecCmd() {
    int number, addr, size, incr;
    char reg[10];
    char cmd;
    char *line = ltrim(cmdLine);

    cmd = *line;

    if (*line) {
        line++;
    }

    switch (cmd) {
        case CMD_DUMP_MEM:
            sscanf(line, "%x", &number);
            addr= number & 0xFFFF;
            
            // Banked Memory, RAM then ROM
            if (addr >= 0xA000) {
                currentBank= (number & 0xFF0000) >> 16;
            }
            
            currentData= addr;
            dumpmode    = DDUMP_RAM;
            break;

        case CMD_DUMP_VERA:
            sscanf(line, "%x", &number);
            addr = number & 0x1FFFF;
            currentData = addr;
            dumpmode    = DDUMP_VERA;
            break;

        case CMD_FILL_MEMORY:
            size = 1;
            sscanf(line, "%x %x %d %d", &addr, &number, &size, &incr);

            if (dumpmode == DDUMP_RAM) {
                addr &= 0xFFFF;
            
                do {
                    if (addr >= 0xC000) {
                        // Nop.
                    }
                    else if (addr >= 0xA000) {
                        RAM[0xa000 + (currentBank << 13) + addr - 0xa000] = number;
                    }
                    else {
                        RAM[addr] = number;
                    }
                    
                    if (incr) {
                        addr += incr;
                    }
                    else {
                        ++addr;
                    }
                    
                    addr &= 0xFFFF;
                    --size;
                } while (size > 0);
            }
            else {
                addr &= 0x1FFFF;
                
                do {
                    video_space_write(addr, number);
                
                    if (incr) {
                        addr += incr;
                    }
                    else {
                        ++addr;
                    }
                    
                    addr &= 0x1FFFF;
                    --size;
                } while (size > 0);
            }
            break;

        case CMD_DISASM:
            sscanf(line, "%x", &number);
            addr = number & 0xFFFF;
            
            // Banked Memory, RAM then ROM
            if(addr >= 0xA000) {
                currentPCBank = (number & 0xFF0000) >> 16;
            }
            
            currentPC = addr;
            break;

        case CMD_SET_BANK:
            sscanf(line, "%s %d", reg, &number);

            if(!strcmp(reg, "rom")) {
                memory_set_rom_bank(number & 0x00FF);
            }
            
            if(!strcmp(reg, "ram")) {
                memory_set_ram_bank(number & 0x00FF);
            }
            break;

        case CMD_SET_REGISTER:
            sscanf(line, "%s %x", reg, &number);

            if(!strcmp(reg, "pc")) {
                pc = number & 0xFFFF;
            }
            
            if(!strcmp(reg, "a")) {
                a = number & 0x00FF;
            }
            
            if(!strcmp(reg, "x")) {
                x = number & 0x00FF;
            }
            
            if(!strcmp(reg, "y")) {
                y = number & 0x00FF;
            }
            
            if(!strcmp(reg, "sp")) {
                sp = number & 0x00FF;
            }
            
            break;

        default:
            break;
    }

    currentPosInLine= currentLineLen = *cmdLine = 0;
}

void DEBUGRenderDisplay(int width, int height) {
    if (showFullDisplay) return;

    SDL_Rect rc;
    rc.w = DBG_WIDTH * 6 * CHAR_SCALE;
    rc.h = height;
    xPos = width-rc.w;yPos = 0;
    rc.x = xPos;rc.y = yPos;
    
    //SDL_SetRenderDrawColor(dbgRenderer,0,0,0,SDL_ALPHA_OPAQUE);
    //SDL_RenderFillRect(dbgRenderer,&rc);

    DEBUGRenderRegisters();
    DEBUGRenderCode(20, currentPC);
    
    if (dumpmode == DDUMP_RAM) {
        DEBUGRenderData(21, currentData);
        DEBUGRenderZeroPageRegisters(21);
    }
    else {
        DEBUGRenderVRAM(21, currentData);
    }
    
    DEBUGRenderStack(20);

    DEBUGRenderCmdLine(xPos, rc.w, height);
}

static void DEBUGRenderCmdLine(int x, int width, int height) {
    char buffer[sizeof(cmdLine)+1];

    //SDL_SetRenderDrawColor(dbgRenderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    //SDL_RenderDrawLine(dbgRenderer, x, height-12, x+width, height-12);

    sprintf(buffer, ">%s", cmdLine);
    DEBUGString(NULL, 0, DBG_HEIGHT-1, buffer, col_cmdLine);
}

static void DEBUGRenderZeroPageRegisters(int y) {
#define LAST_R 15
    int reg = 0;
    int y_start = y;
    char lbl[6];
    
    while (reg < DBGMAX_ZERO_PAGE_REGISTERS) {
        if (((y-y_start) % 5) != 0) {           // Break registers into groups of 5, easier to locate
            if (reg <= LAST_R)
                sprintf(lbl, "R%d", reg);
            else
                sprintf(lbl, "x%d", reg);

            DEBUGString(NULL, DBG_ZP_REG, y, lbl, col_label);

            int reg_addr = 2 + reg * 2;
            int n = real_read6502(reg_addr+1, true, currentBank)*256+real_read6502(reg_addr, true, currentBank);

            DEBUGNumber(DBG_ZP_REG+5, y, n, 4, col_data);

            if (oldRegChange[reg] != NULL)
                DEBUGString(NULL, DBG_ZP_REG+9, y, oldRegChange[reg], col_data);

            if (oldRegisterTicks != clockticks6502) {   // change detection only when the emulated CPU changes
                oldRegChange[reg] = n != oldRegisters[reg] ? "*" : " ";
                oldRegisters[reg]=n;
            }
            
            reg++;
        }
        
        y++;
    }

    if (oldRegisterTicks != clockticks6502) {
        oldRegisterTicks = clockticks6502;
    }
}


static void DEBUGRenderData(int y,int data) {
    while (y < DBG_HEIGHT-2) {
        DEBUGAddress(DBG_MEMX, y, (uint8_t)currentBank, data & 0xFFFF, col_label);

        for (int i = 0;i < 8;i++) {
            int byte= real_read6502((data+i) & 0xFFFF, true, currentBank);
            DEBUGNumber(DBG_MEMX+8+i*3,y,byte,2, col_data);
            DEBUGWrite(NULL, DBG_MEMX+33+i,y,byte, col_data);
        }
        
        y++;
        data += 8;
    }
}

static void DEBUGRenderVRAM(int y, int data) {
    while (y < DBG_HEIGHT - 2) {
        DEBUGVAddress(DBG_MEMX, y, data & 0x1FFFF, col_label);

        for (int i = 0; i < 16; i++) {
            int addr = (data + i) & 0x1FFFF;
            int byte = video_space_read(addr);

            if (video_is_tilemap_address(addr)) {
                DEBUGNumber(DBG_MEMX + 6 + i * 3, y, byte, 2, col_vram_tilemap);
            }
            else if (video_is_tiledata_address(addr)) {
                DEBUGNumber(DBG_MEMX + 6 + i * 3, y, byte, 2, col_vram_tiledata);
            }
            else if (video_is_special_address(addr)) {
                DEBUGNumber(DBG_MEMX + 6 + i * 3, y, byte, 2, col_vram_special);
            }
            else {
                DEBUGNumber(DBG_MEMX + 6 + i * 3, y, byte, 2, col_vram_other);
            }
        }
        
        y++;
        data += 16;
    }
}

static void DEBUGRenderCode(int lines, int initialPC) {
    char buffer[32];
    
    for (int y = 0; y < lines; y++) {
        DEBUGAddress(DBG_ASMX, y, currentPCBank, initialPC, col_label);

        int size = disasm(initialPC, RAM, buffer, sizeof(buffer), true, currentPCBank);
        DEBUGString(NULL, DBG_ASMX+8, y, buffer, initialPC == pc ? col_highlight : col_data);
        initialPC += size;
    }
}

static char *labels[] = { "NV-BDIZC","","","A","X","Y","","BKA","BKO", "PC","SP","","BRK","", "VA","VD0","VD1","VCT", NULL };

static int DEBUGRenderRegisters(void) {
    int n = 0,yc = 0;
    
    while (labels[n] != NULL) {
        DEBUGString(NULL, DBG_LBLX,n,labels[n], col_label);n++;
    }
    
    yc++;
    DEBUGNumber(DBG_LBLX, yc, (status >> 7) & 1, 1, col_data);
    DEBUGNumber(DBG_LBLX+1, yc, (status >> 6) & 1, 1, col_data);
    DEBUGNumber(DBG_LBLX+3, yc, (status >> 4) & 1, 1, col_data);
    DEBUGNumber(DBG_LBLX+4, yc, (status >> 3) & 1, 1, col_data);
    DEBUGNumber(DBG_LBLX+5, yc, (status >> 2) & 1, 1, col_data);
    DEBUGNumber(DBG_LBLX+6, yc, (status >> 1) & 1, 1, col_data);
    DEBUGNumber(DBG_LBLX+7, yc, (status >> 0) & 1, 1, col_data);
    yc+= 2;

    DEBUGNumber(DBG_DATX, yc++, a, 2, col_data);
    DEBUGNumber(DBG_DATX, yc++, x, 2, col_data);
    DEBUGNumber(DBG_DATX, yc++, y, 2, col_data);
    yc++;

    DEBUGNumber(DBG_DATX, yc++, memory_get_ram_bank(), 2, col_data);
    DEBUGNumber(DBG_DATX, yc++, memory_get_rom_bank(), 2, col_data);
    DEBUGNumber(DBG_DATX, yc++, pc, 4, col_data);
    DEBUGNumber(DBG_DATX, yc++, sp|0x100, 4, col_data);
    yc++;

    DEBUGNumber(DBG_DATX, yc++, breakPoint & 0xFFFF, 4, col_data);
    yc++;

    DEBUGNumber(DBG_DATX, yc++, video_read(0, true) | (video_read(1, true)<<8) | (video_read(2, true)<<16), 2, col_data);
    DEBUGNumber(DBG_DATX, yc++, video_read(3, true), 2, col_data);
    DEBUGNumber(DBG_DATX, yc++, video_read(4, true), 2, col_data);
    DEBUGNumber(DBG_DATX, yc++, video_read(5, true), 2, col_data);

    return n;
}


static void DEBUGRenderStack(int bytesCount) {
    int data= (sp+1) | 0x100;
    int y= 0;

    while (y < bytesCount) {
        DEBUGNumber(DBG_STCK,y,data & 0xFFFF,4, col_label);

        int byte = real_read6502((data++) & 0xFFFF, false, 0);
        DEBUGNumber(DBG_STCK+5,y,byte,2, col_data);
        DEBUGWrite(NULL, DBG_STCK+9,y,byte, col_data);

        y++;
        data= (data & 0xFF) | 0x100;
    }
}

static void DEBUGNumber(int x, int y, int n, int w, SDL_Color colour) {
    char fmtString[8],buffer[16];

    snprintf(fmtString, sizeof(fmtString), "%%0%dX", w);
    snprintf(buffer, sizeof(buffer), fmtString, n);
    DEBUGString(NULL, x, y, buffer, colour);
}

static void DEBUGAddress(int x, int y, int bank, int addr, SDL_Color colour) {
    char buffer[4];

    if(addr >= 0xA000) {
        snprintf(buffer, sizeof(buffer), "%.2X:", bank);
    }
    else {
        strcpy(buffer, "--:");
    }

    DEBUGString(NULL, x, y, buffer, colour);
    DEBUGNumber(x+3, y, addr, 4, colour);
}

static void DEBUGVAddress(int x, int y, int addr, SDL_Color colour) {
    DEBUGNumber(x, y, addr, 5, colour);
}
