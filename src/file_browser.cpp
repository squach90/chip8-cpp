#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <termios.h>
#include <unistd.h>
#include "file_browser.hpp"

namespace fs = std::filesystem;

std::string get_key() {
    termios oldt{}, newt{};
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    read(STDIN_FILENO, &ch, 1);
    if (ch == '\x1b') {
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) == 0) return "\x1b";
        if (read(STDIN_FILENO, &seq[1], 1) == 0) return "\x1b";
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return std::string("\x1b") + seq[0] + seq[1];
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return std::string(1, ch);
}

std::string select_rom_file() {
    fs::path current_path = fs::current_path();
    int selected_index = 0;

    while (true) {
        std::vector<std::pair<std::string, bool>> items;

        // Ajouter ".."
        if (current_path.has_parent_path()) {
            items.emplace_back("..", true);
        }

        for (auto &entry : fs::directory_iterator(current_path)) {
            bool is_dir = entry.is_directory();
            items.emplace_back(entry.path().filename().string(), is_dir);
        }

        system("clear");
        std::cout << "📂 CHIP-8 ROM SELECTOR\n";
        std::cout << "Path: " << current_path << "\n";
        std::cout << "↑/↓ Navigate | Enter = Open \n";
        std::cout << "----------------------------------------\n";

        for (int i = 0; i < (int)items.size(); ++i) {
            std::cout << (i == selected_index ? "> " : "  ")
                      << (items[i].second ? "[DIR] " : "[ROM] ")
                      << items[i].first << "\n";
        }

        std::string key = get_key();

        if (key == "\x1b[A") { // flèche haut
            selected_index = (selected_index - 1 + items.size()) % items.size();
        } else if (key == "\x1b[B") { // flèche bas
            selected_index = (selected_index + 1) % items.size();
        } else if (key == "\n" || key == "\r") { // Entrée
            auto [name, is_dir] = items[selected_index];
            if (is_dir) {
                if (name == "..")
                    current_path = current_path.parent_path();
                else
                    current_path /= name;
                selected_index = 0;
            } else {
                return (current_path / name).string();
            }
        }
    }
}
