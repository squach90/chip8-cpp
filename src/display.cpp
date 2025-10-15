#include "display.hpp"
#include <stdexcept>

Display::Display(int width, int height, int scale)
    : width(width), height(height), scale(scale) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("SDL could not initialize!");
    }
    window = SDL_CreateWindow("Chip-8 Emulator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width * scale, height * scale, SDL_WINDOW_SHOWN);
    if (!window) {
        throw std::runtime_error("Window could not be created!");
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        throw std::runtime_error("Renderer could not be created!");
    }
}

Display::~Display() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Display::draw(const Chip8& chip8) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (chip8.gfx[x + y * width]) {
                SDL_Rect rect = {x * scale, y * scale, scale, scale};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

void Display::update() {
    SDL_RenderPresent(renderer);
}

void Display::handle_events(Chip8& chip8, const std::unordered_map<int, uint8_t>& key_map) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            exit(0);
        } else if (e.type == SDL_KEYDOWN) {
            auto it = key_map.find(e.key.keysym.sym);
            if (it != key_map.end()) {
                chip8.key[it->second] = 1;
            }
        } else if (e.type == SDL_KEYUP) {
            auto it = key_map.find(e.key.keysym.sym);
            if (it != key_map.end()) {
                chip8.key[it->second] = 0;
            }
        }
    }
}
