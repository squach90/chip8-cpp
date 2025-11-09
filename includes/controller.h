#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <SDL.h>
#include "cpu.h"

extern int debug;

void controller_init();

void controller_update(Chip8 *chip);

int controller_quit();

#endif
