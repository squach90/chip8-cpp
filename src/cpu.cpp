#include <fstream>
#include <cstdint>
#include <string>
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <random>
#include "file_browser.hpp"
#include "cpu.hpp"
using namespace std;

int main() {
    std::cout << "🎮 Chip-8 Emulator\n";
    std::cout << "Select a ROM...\n";

    std::string rom = select_rom_file();
    if (rom.empty()) {
        std::cout << "❌ No ROM selected. Closing...\n";
        return 0;
    }

    std::cout << "✅ Loading of: " << rom << "\n";

    Chip8 chip;
    try {
        chip.load_program(rom);
    } catch (const std::exception& e) {
        std::cerr << "❌ Error when loading ROM: " << e.what() << "\n";
        return 1;
    }

    bool running = true;
    int timer_counter = 0;

    std::cout << "🚀 Emulation Start\n";
    std::cout << "Press Ctrl+C to quit\n";

    while (running) {
        chip.emulation_cycle();

        timer_counter++;
        if (timer_counter >= 10) {
            chip.update_timers();
            timer_counter = 0;
        }

        if (chip.draw_flag) {
            // Ici tu pourrais appeler ton rendu graphique, ex:
            // window.draw(chip);
            chip.draw_flag = false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "👋 Closing the emulator...\n";

    return 0;
}