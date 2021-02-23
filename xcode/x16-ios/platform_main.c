//
//  platform_main.c
//  Replaces main.c from original Commander X16 emulator by Michael Steil
//
//  Written 2021 by Roger Boesch
//  "You can do whatever you like with it"
//

#ifndef __APPLE__
#define _XOPEN_SOURCE   600
#define _POSIX_C_SOURCE 1
#endif
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#ifdef __MINGW32__
#include <ctype.h>
#endif
#include "../../cpu/fake6502.h"
#include "disasm.h"
#include "memory.h"
#include "video.h"
#include "via.h"
#include "ps2.h"
#include "spi.h"
#include "vera_spi.h"
#include "sdcard.h"
#include "loadsave.h"
#include "glue.h"
#include "debugger.h"
#include "utf8.h"
#include "joystick.h"
#include "utf8_encode.h"
#include "rom_symbols.h"
#include "ym2151.h"
#include "audio.h"
#include "version.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <pthread.h>
#endif

#include <ctype.h>

void *emulator_loop(void *param);
void emscripten_main_loop(void);

// This must match the KERNAL's set!
char *keymaps[] = {
    "en-us",
    "en-gb",
    "de",
    "nordic",
    "it",
    "pl",
    "hu",
    "es",
    "fr",
    "de-ch",
    "fr-be",
    "pt-br",
};

#ifdef PERFSTAT
uint32_t stat[65536];
#endif

bool debugger_enabled = false;
char *paste_text = NULL;
char paste_text_data[65536];
bool pasting_bas = false;

char sdcard_path[256];
bool use_sdcard = false;

uint16_t num_ram_banks = 64; // 512 KB default

bool run_geos = false;
bool log_video = false;
bool log_speed = false;
bool log_keyboard = false;
bool dump_cpu = false;
bool dump_ram = true;
bool dump_bank = true;
bool dump_vram = false;
bool warp_mode = false;
echo_mode_t echo_mode;
bool save_on_exit = true;
gif_recorder_state_t record_gif = RECORD_GIF_DISABLED;
char *gif_path = NULL;
uint8_t keymap = 0; // KERNAL's default
int window_scale = 1;
char *scale_quality = "best";

int frames;
double sdlTicks_base;
int32_t last_perf_update;
int32_t perf_frame_count;
char window_title[30];

#ifdef TRACE
bool trace_mode = false;
uint16_t trace_address = 0;
#endif

int instruction_counter;
SDL_RWops *prg_file ;
int prg_override_start = -1;
bool run_after_load = false;

#ifdef TRACE
#include "rom_labels.h"
char *
label_for_address(uint16_t address) {
    uint16_t *addresses;
    char **labels;
    int count;
    switch (memory_get_rom_bank()) {
        case 0:
            addresses = addresses_bank0;
            labels = labels_bank0;
            count = sizeof(addresses_bank0) / sizeof(uint16_t);
            break;
        case 1:
            addresses = addresses_bank1;
            labels = labels_bank1;
            count = sizeof(addresses_bank1) / sizeof(uint16_t);
            break;
        case 2:
            addresses = addresses_bank2;
            labels = labels_bank2;
            count = sizeof(addresses_bank2) / sizeof(uint16_t);
            break;
        case 3:
            addresses = addresses_bank3;
            labels = labels_bank3;
            count = sizeof(addresses_bank3) / sizeof(uint16_t);
            break;
        case 4:
            addresses = addresses_bank4;
            labels = labels_bank4;
            count = sizeof(addresses_bank4) / sizeof(uint16_t);
            break;
        case 5:
            addresses = addresses_bank5;
            labels = labels_bank5;
            count = sizeof(addresses_bank5) / sizeof(uint16_t);
            break;
        case 6:
            addresses = addresses_bank6;
            labels = labels_bank6;
            count = sizeof(addresses_bank6) / sizeof(uint16_t);
            break;
        default:
            addresses = NULL;
            labels = NULL;
    }

    if (!addresses) {
        return NULL;
    }

    for (int i = 0; i < count; i++) {
        if (address == addresses[i]) {
            return labels[i];
        }
    }
    
    return NULL;
}
#endif

void machine_dump() {
    int index = 0;
    char filename[22];
    for (;;) {
        if (!index) {
            strcpy(filename, "dump.bin");
        } else {
            sprintf(filename, "dump-%i.bin", index);
        }
        if (access(filename, F_OK) == -1) {
            break;
        }
        index++;
    }
    SDL_RWops *f = SDL_RWFromFile(filename, "wb");
    if (!f) {
        printf("Cannot write to %s!\n", filename);
        return;
    }

    if (dump_cpu) {
        SDL_RWwrite(f, &a, sizeof(uint8_t), 1);
        SDL_RWwrite(f, &x, sizeof(uint8_t), 1);
        SDL_RWwrite(f, &y, sizeof(uint8_t), 1);
        SDL_RWwrite(f, &sp, sizeof(uint8_t), 1);
        SDL_RWwrite(f, &status, sizeof(uint8_t), 1);
        SDL_RWwrite(f, &pc, sizeof(uint16_t), 1);
    }
    memory_save(f, dump_ram, dump_bank);

    if (dump_vram) {
        video_save(f);
    }

    SDL_RWclose(f);
    printf("Dumped system to %s.\n", filename);
}

void machine_reset() {
    vera_spi_init();
    via1_init();
    via2_init();
    video_reset();
    reset6502();
}

void machine_paste(char *s) {
    if (s) {
        paste_text = s;
        pasting_bas = true;
    }
}

void machine_copy() {
    // Implement save and copy to clipboard
}

void machine_set_sd_card(char* path) {
    strcpy(sdcard_path, path);
    use_sdcard = true;
}

void machine_attach_sdcard() {
    if (!use_sdcard) {
        return;
    }
    
    if (sdcard_file != NULL) {
        sdcard_detach();
    }
    
    sdcard_file = SDL_RWFromFile(sdcard_path, "r+b");

    if (!sdcard_file) {
        printf("Cannot open %s!\n", sdcard_path);
        return;
    }

    sdcard_attach();
}

void machine_deattach_sdcard() {
    if (sdcard_file != NULL) {
        sdcard_detach();
    }
    
    sdcard_file = NULL;
}

void timing_init() {
    frames = 0;
    sdlTicks_base = platform_get_ticks();
    last_perf_update = 0;
    perf_frame_count = 0;
}

void timing_update() {
    frames++;
    double sdlTicks = platform_get_ticks() - sdlTicks_base;
    double diff_time = 1000 * frames / 60 - sdlTicks;
   
    if (!warp_mode && diff_time > 0) {
        usleep(1000 * diff_time);
    }

    if (sdlTicks - last_perf_update > 5000) {
        int32_t frameCount = frames - perf_frame_count;
        int perf = frameCount / 3;

        if (perf < 100 || warp_mode) {
            sprintf(window_title, "Commander X16 (%d%%)", perf);
            video_update_title(window_title);
        } else {
            video_update_title("Commander X16");
        }

        perf_frame_count = frames;
        last_perf_update = sdlTicks;
    }

    if (log_speed) {
        float frames_behind = -((float)diff_time / 16.666666);
        int load = (int)((1 + frames_behind) * 100);
        printf("Load: %d%%\n", load > 100 ? 100 : load);

        if ((int)frames_behind > 0) {
            printf("Rendering is behind %d frames.\n", -(int)frames_behind);
        }
    }
}

void machine_toggle_warp() {
    warp_mode = !warp_mode;
    timing_init();
}

void machine_toggle_geos() {
    run_geos = !run_geos;
    machine_reset();
    
    if (run_geos) {
        paste_text = "GEOS\r";
    }
    else {
        paste_text = "";
    }
}

uint8_t iso8859_15_from_unicode(uint32_t c) {
    // line feed -> carriage return
    if (c == '\n') {
        return '\r';
    }

    // translate Unicode characters not part of Latin-1 but part of Latin-15
    switch (c) {
        case 0x20ac: // '€'
            return 0xa4;
        case 0x160: // 'Š'
            return 0xa6;
        case 0x161: // 'š'
            return 0xa8;
        case 0x17d: // 'Ž'
            return 0xb4;
        case 0x17e: // 'ž'
            return 0xb8;
        case 0x152: // 'Œ'
            return 0xbc;
        case 0x153: // 'œ'
            return 0xbd;
        case 0x178: // 'Ÿ'
            return 0xbe;
    }

    // remove Unicode characters part of Latin-1 but not part of Latin-15
    switch (c) {
        case 0xa4: // '¤'
        case 0xa6: // '¦'
        case 0xa8: // '¨'
        case 0xb4: // '´'
        case 0xb8: // '¸'
        case 0xbc: // '¼'
        case 0xbd: // '½'
        case 0xbe: // '¾'
            return '?';
    }

    // all other Unicode characters are also unsupported
    if (c >= 256) {
        return '?';
    }

    // everything else is Latin-15 already
    return c;
}

uint32_t unicode_from_iso8859_15(uint8_t c) {
    // translate Latin-15 characters not part of Latin-1
    switch (c) {
        case 0xa4:
            return 0x20ac; // '€'
        case 0xa6:
            return 0x160; // 'Š'
        case 0xa8:
            return 0x161; // 'š'
        case 0xb4:
            return 0x17d; // 'Ž'
        case 0xb8:
            return 0x17e; // 'ž'
        case 0xbc:
            return 0x152; // 'Œ'
        case 0xbd:
            return 0x153; // 'œ'
        case 0xbe:
            return 0x178; // 'Ÿ'
        default:
            return c;
    }
}

// converts the character to UTF-8 and prints it
static void print_iso8859_15_char(char c) {
    char utf8[5];
    utf8_encode(utf8, unicode_from_iso8859_15(c));
    printf("%s", utf8);
}

static bool is_kernal() {
    return read6502(0xfff6) == 'M' && // only for KERNAL
            read6502(0xfff7) == 'I' &&
            read6502(0xfff8) == 'S' &&
            read6502(0xfff9) == 'T';
}

int platform_main(bool record) {
    if (record) {
        record_gif = RECORD_GIF_PAUSED;
        gif_path = platform_get_gif_path();
    }

    char *rom_filename = "rom.bin";
    char rom_path_data[PATH_MAX];
    debugger_enabled = true;

    char *rom_path = rom_path_data;
    char *prg_path = NULL;
    char *bas_path = NULL;
    bool run_test = false;
    int test_number = 0;
    int audio_buffers = 8;

    const char *audio_dev_name = NULL;

    run_after_load = false;

    char *base_path = platform_get_base_path();

    // This causes the emulator to load ROM data from the executable's directory when
    // no ROM file is specified on the command line.
    memcpy(rom_path, base_path, strlen(base_path) + 1);
    strncpy(rom_path + strlen(rom_path), rom_filename, PATH_MAX - strlen(rom_path));

    SDL_RWops *f = SDL_RWFromFile(rom_path, "rb");
    if (!f) {
        printf("Cannot open %s!\n", rom_path);
        exit(1);
    }
    
    size_t rom_size = SDL_RWread(f, ROM, ROM_SIZE, 1);
    (void)rom_size;
    SDL_RWclose(f);

    if (use_sdcard) {
        machine_attach_sdcard();
    }

    prg_override_start = -1;
    if (prg_path) {
        char *comma = strchr(prg_path, ',');
        if (comma) {
            prg_override_start = (uint16_t)strtol(comma + 1, NULL, 16);
            *comma = 0;
        }

        prg_file = SDL_RWFromFile(prg_path, "rb");
        if (!prg_file) {
            printf("Cannot open %s!\n", prg_path);
            exit(1);
        }
    }

    if (bas_path) {
        SDL_RWops *bas_file = SDL_RWFromFile(bas_path, "r");
        if (!bas_file) {
            printf("Cannot open %s!\n", bas_path);
            exit(1);
        }
        paste_text = paste_text_data;
        size_t paste_size = SDL_RWread(bas_file, paste_text, 1, sizeof(paste_text_data) - 1);
        if (run_after_load) {
            strncpy(paste_text + paste_size, "\rRUN\r", sizeof(paste_text_data) - paste_size);
        } else {
            paste_text[paste_size] = 0;
        }
        SDL_RWclose(bas_file);
    }

    if (run_geos) {
        paste_text = "GEOS\r";
    }
    
    if (run_test) {
        paste_text = paste_text_data;
        snprintf(paste_text, sizeof(paste_text_data), "TEST %d\r", test_number);
    }

    audio_init(audio_dev_name, audio_buffers);

    memory_init();
    video_init(window_scale, scale_quality);

    joystick_init();

    machine_reset();

    timing_init();

    instruction_counter = 0;

    emulator_loop(NULL);

    audio_close();
    video_end();

#ifdef PERFSTAT
    for (int pc = 0xc000; pc < sizeof(stat)/sizeof(*stat); pc++) {
        if (stat[pc] == 0) {
            continue;
        }
        char *label = label_for_address(pc);
        if (!label) {
            continue;
        }
        char *original_label = label;
        uint16_t pc2 = pc;
        if (label[0] == '@') {
            label = NULL;
            while (!label || label[0] == '@') {
                pc2--;
                label = label_for_address(pc2);
            }
        }
        printf("%d\t $%04X %s+%d", stat[pc], pc, label, pc-pc2);
        if (pc-pc2 != 0) {
            printf(" (%s)", original_label);
        }
        printf("\n");
    }
#endif

    return 0;
}

void* emulator_loop(void *param) {
    for (;;) {

        if (debugger_enabled) {
            int dbgCmd = DEBUGGetCurrentStatus();
            if (dbgCmd > 0) continue;
            if (dbgCmd < 0) break;
        }

#ifdef PERFSTAT

        if (memory_get_rom_bank() == 3) {
            static uint8_t old_sp;
            static uint16_t base_pc;
            if (sp < old_sp) {
                base_pc = pc;
            }
            old_sp = sp;
            stat[base_pc]++;
        }
#endif

#ifdef TRACE
        if (pc == trace_address && trace_address != 0) {
            trace_mode = true;
        }
        
        if (trace_mode) {
            //printf("\t\t\t\t");
            printf("[%6d] ", instruction_counter);

            char *label = label_for_address(pc);
            int label_len = label ? strlen(label) : 0;
            if (label) {
                printf("%s", label);
            }
            for (int i = 0; i < 20 - label_len; i++) {
                printf(" ");
            }
            printf(" %02x:.,%04x ", memory_get_rom_bank(), pc);
            char disasm_line[15];
            int len = disasm(pc, RAM, disasm_line, sizeof(disasm_line), false, 0);
            for (int i = 0; i < len; i++) {
                printf("%02x ", read6502(pc + i));
            }
            for (int i = 0; i < 9 - 3 * len; i++) {
                printf(" ");
            }
            printf("%s", disasm_line);
            for (int i = 0; i < 15 - strlen(disasm_line); i++) {
                printf(" ");
            }

            printf("a=$%02x x=$%02x y=$%02x s=$%02x p=", a, x, y, sp);
            for (int i = 7; i >= 0; i--) {
                printf("%c", (status & (1 << i)) ? "czidb.vn"[i] : '-');
            }

#if 0
            printf(" ---");
            for (int i = 0; i < 6; i++) {
                printf(" r%i:%04x", i, RAM[2 + i*2] | RAM[3 + i*2] << 8);
            }
            for (int i = 14; i < 16; i++) {
                printf(" r%i:%04x", i, RAM[2 + i*2] | RAM[3 + i*2] << 8);
            }

            printf(" RAM:%01x", memory_get_ram_bank());
            printf(" px:%d py:%d", RAM[0xa0e8] | RAM[0xa0e9] << 8, RAM[0xa0ea] | RAM[0xa0eb] << 8);
#endif

            printf("\n");
        }
#endif

#ifdef LOAD_HYPERCALLS
        if ((pc == 0xffd5 || pc == 0xffd8) && is_kernal() && RAM[FA] == 8 && !sdcard_file) {
            if (pc == 0xffd5) {
                LOAD();
            } else {
                SAVE();
            }
            pc = (RAM[0x100 + sp + 1] | (RAM[0x100 + sp + 2] << 8)) + 1;
            sp += 2;
        }
#endif

        uint32_t old_clockticks6502 = clockticks6502;
        step6502();
        uint8_t clocks = clockticks6502 - old_clockticks6502;
        bool new_frame = false;
        
        for (uint8_t i = 0; i < clocks; i++) {
            ps2_step(0);
            ps2_step(1);
            joystick_step();
            vera_spi_step();
            new_frame |= video_step(MHZ);
        }
        
        audio_render(clocks);

        instruction_counter++;
        
        if (new_frame) {
            if (!video_update()) {
                break;
            }

            timing_update();
        }

        if (video_get_irq_out()) {
            if (!(status & 4)) {
                irq6502();
            }
        }

        if (pc == 0xffff) {
            if (save_on_exit) {
                machine_dump();
            }
            
            break;
        }

        if (echo_mode != ECHO_MODE_NONE && pc == 0xffd2 && is_kernal()) {
            uint8_t c = a;
            
            if (echo_mode == ECHO_MODE_COOKED) {
                if (c == 0x0d) {
                    printf("\n");
                }
                else if (c == 0x0a) {
                    // skip
                }
                else if (c < 0x20 || c >= 0x80) {
                    printf("\\X%02X", c);
                }
                else {
                    printf("%c", c);
                }
            }
            else if (echo_mode == ECHO_MODE_ISO) {
                if (c == 0x0d) {
                    printf("\n");
                }
                else if (c == 0x0a) {
                    // skip
                }
                else if (c < 0x20 || (c >= 0x80 && c < 0xa0)) {
                    printf("\\X%02X", c);
                }
                else {
                    print_iso8859_15_char(c);
                }
            }
            else {
                printf("%c", c);
            }
            
            fflush(stdout);
        }

        if (pc == 0xffcf && is_kernal()) {
            // as soon as BASIC starts reading a line...
            if (prg_file) {
                // ...inject the app into RAM
                uint8_t start_lo = SDL_ReadU8(prg_file);
                uint8_t start_hi = SDL_ReadU8(prg_file);
                uint16_t start;
                
                if (prg_override_start >= 0) {
                    start = prg_override_start;
                }
                else {
                    start = start_hi << 8 | start_lo;
                }
                
                uint16_t end = start + SDL_RWread(prg_file, RAM + start, 1, 65536-start);
                SDL_RWclose(prg_file);
                prg_file = NULL;
                
                if (start == 0x0801) {
                    // set start of variables
                    RAM[VARTAB] = end & 0xff;
                    RAM[VARTAB + 1] = end >> 8;
                }

                if (run_after_load) {
                    if (start == 0x0801) {
                        paste_text = "RUN\r";
                    }
                    else {
                        paste_text = paste_text_data;
                        snprintf(paste_text, sizeof(paste_text_data), "SYS$%04X\r", start);
                    }
                }
            }

            if (paste_text) {
                // ...paste BASIC code into the keyboard buffer
                pasting_bas = true;
            }
        }

        while (pasting_bas && RAM[NDX] < 10) {
            uint32_t c;
            int e = 0;

            if (paste_text[0] == '\\' && paste_text[1] == 'X' && paste_text[2] && paste_text[3]) {
                uint8_t hi = strtol((char[]){paste_text[2], 0}, NULL, 16);
                uint8_t lo = strtol((char[]){paste_text[3], 0}, NULL, 16);
                c = hi << 4 | lo;
                paste_text += 4;
            }
            else {
                paste_text = utf8_decode(paste_text, &c, &e);
                c = iso8859_15_from_unicode(c);
            }
            
            if (c && !e) {
                RAM[KEYD + RAM[NDX]] = c;
                RAM[NDX]++;
            }
            else {
                pasting_bas = false;
                paste_text = NULL;
            }
        }
    }

    return 0;
}
