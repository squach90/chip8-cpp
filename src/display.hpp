#pragma once
#include <SDL2/SDL.h>
#include <cstdint>
#include "cpu.hpp"

class Display {
public:
    Display(int width, int height, int scale);
    ~Display();
    void draw(const Chip8& chip8);
    void update();
    void handle_events(Chip8& chip8, const std::unordered_map<int, uint8_t>& key_map);

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    int width;
    int height;
    int scale;
};
