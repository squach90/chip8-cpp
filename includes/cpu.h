#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t memory[4096];
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
    uint8_t gfx[64 * 32];
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t stack[16];
    uint16_t sp;
    uint8_t key[16];
    bool draw_flag;
} Chip8;

extern int debug;

void chip8_init(Chip8 *chip);
int chip8_load_program(Chip8 *chip, const char *filename);
void chip8_emulation_cycle(Chip8 *chip);
void chip8_update_timers(Chip8 *chip);

#endif
