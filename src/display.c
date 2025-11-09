#include <SDL.h>
#include "../includes/display.h"

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCREEN_SCALING 16

struct State {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
} state;

void createWindow() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Erreur SDL_Init: %s", SDL_GetError());
        return;
    }

    state.window = SDL_CreateWindow("Chip-8 Emulator",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * SCREEN_SCALING, SCREEN_HEIGHT * SCREEN_SCALING,
        SDL_WINDOW_SHOWN);

    if (!state.window) {
        SDL_Log("Erreur SDL_CreateWindow: %s", SDL_GetError());
        return;
    }

    state.renderer = SDL_CreateRenderer(state.window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!state.renderer) {
        SDL_Log("Erreur SDL_CreateRenderer: %s", SDL_GetError());
        return;
    }

    state.texture = SDL_CreateTexture(state.renderer,
        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT);

    if (!state.texture) {
        SDL_Log("Erreur SDL_CreateTexture: %s", SDL_GetError());
        return;
    }
}

void draw_screen(Chip8 *chip) {
    if (!chip->draw_flag) return;
    chip->draw_flag = false;

    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 255);
    SDL_RenderClear(state.renderer);

    SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (chip->gfx[x + y * 64]) {
                SDL_Rect pixel = { x * SCREEN_SCALING, y * SCREEN_SCALING, SCREEN_SCALING, SCREEN_SCALING };
                SDL_RenderFillRect(state.renderer, &pixel);
            }
        }
    }

    SDL_RenderPresent(state.renderer);
}


void destroyWindow() {
    if (state.texture) SDL_DestroyTexture(state.texture);
    if (state.renderer) SDL_DestroyRenderer(state.renderer);
    if (state.window) SDL_DestroyWindow(state.window);
    SDL_Quit();
}
