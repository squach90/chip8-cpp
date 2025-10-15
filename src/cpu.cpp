#include <iostream>
#include <thread>
#include <chrono>
#include <unordered_map>
#include "cpu.hpp"
#include "file_browser.hpp"

int main() {
    std::unordered_map<int, uint8_t> key_map = {
        { '1', 0x1 }, { '2', 0x2 }, { '3', 0x3 }, { '4', 0xC },
        { 'q', 0x4 }, { 'w', 0x5 }, { 'e', 0x6 }, { 'r', 0xD },
        { 'a', 0x7 }, { 's', 0x8 }, { 'd', 0x9 }, { 'f', 0xE },
        { 'z', 0xA }, { 'x', 0x0 }, { 'c', 0xB }, { 'v', 0xF }
    };

    std::cout << "🎮 Chip-8 Emulator (console mode)\n";
    std::string rom = select_rom_file();
    if (rom.empty()) {
        std::cout << "❌ No ROM selected. Closing...\n";
        return 0;
    }

    Chip8 chip;
    try {
        chip.load_program(rom);
    } catch (const std::exception& e) {
        std::cerr << "❌ Error loading ROM: " << e.what() << "\n";
        return 1;
    }

    std::cout << "🚀 Emulation started. Press Ctrl+C to quit.\n";

    int timer_counter = 0;

    while (true) {
        chip.emulation_cycle();

        timer_counter++;
        if (timer_counter >= 10) {
            chip.update_timers();
            timer_counter = 0;
        }

        if (chip.draw_flag) {
            #ifdef _WIN32
            system("cls");
            #else
            system("clear");
            #endif
            chip.print_screen();
            chip.draw_flag = false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }


    return 0;
}
