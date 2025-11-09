#include "../includes/controller.h"
#include <stdbool.h>

static bool quit = false;

// Mapping SDL_Keycode to CHIP-8 keys
static const SDL_Keycode keymap[16] = {
    SDLK_x,  // 0
    SDLK_1,  // 1
    SDLK_2,  // 2
    SDLK_3,  // 3
    SDLK_q,  // 4
    SDLK_w,  // 5
    SDLK_e,  // 6
    SDLK_a,  // 7
    SDLK_s,  // 8
    SDLK_d,  // 9
    SDLK_z,  // A
    SDLK_c,  // B
    SDLK_4,  // C
    SDLK_r,  // D
    SDLK_f,  // E
    SDLK_v   // F
};

void controller_init() {
    SDL_InitSubSystem(SDL_INIT_EVENTS);
}

void controller_update(Chip8 *chip) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit = true;
        }
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            bool pressed = (event.type == SDL_KEYDOWN);
            for (int i = 0; i < 16; i++) {
                if (event.key.keysym.sym == keymap[i]) {
                    chip->key[i] = pressed;
                    if (debug) {
                        printf("Key %X %s\n", i, pressed ? "pressed" : "released");
                    }
                }
            }
        }

    }
}

int controller_quit() {
    return quit;
}
