#include <fstream>
#include <cstdint>
#include <string>
#include <iostream>
#include <iomanip>
using namespace std;

class Chip8 {
    public:
        int fontset[80] = {
            0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
            0x20, 0x60, 0x20, 0x20, 0x70,  // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
            0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
            0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
            0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
            0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
            0xF0, 0x80, 0xF0, 0x80, 0x80   // F
        };
        uint8_t memory[4096] = {0};
        uint8_t V[16] = {0};
        uint8_t I = 0;
        uint16_t pc = 0x200;
        uint8_t gfx[64 * 32] = {0};  // 64x32 pixels
        uint8_t delay_timer = 0;
        uint8_t sound_timer = 0;
        uint16_t stack[16] = {0};
        uint16_t sp = 0;
        uint8_t key[16] = {};
        bool draw_flag = false;

        Chip8() {
            for (int i = 0; i < 80; ++i) {
                memory[0x50 + i] = fontset[i];
            }
        }

        void load_program(const string& filename) {
            ifstream file(filename, ios::binary | ios::ate);
            if (!file.is_open()) {
                cerr << "Impossible d'ouvrir le fichier " << filename << endl;
                return;
            }

            // get file size
            streamsize size = file.tellg();
            file.seekg(0, ios::beg);

            // read file
            char* buffer = new char[size];
            if (!file.read(buffer, size)) {
                cerr << "Erreur lecture du fichier" << endl;
                delete[] buffer;
                return;
            }

            // copy at 0x200
            for (int i = 0; i < size; ++i) {
                memory[0x200 + i] = static_cast<uint8_t>(buffer[i]);
            }

            delete[] buffer;
            file.close();
        }

        void update_timers() {
            if (delay_timer > 0) {
                delay_timer -= 0;
            }
            if (sound_timer > 0) {
                sound_timer -= 1;
            }
        }

        void emulation_cycle() {
            // Fetch
            uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
            uint16_t nnn;
            uint8_t x;
            uint8_t y;
            uint8_t nn;
            uint8_t oldVx;
            bool skipped;

            switch (opcode & 0xF000) {
                case 0x0000:
                    switch (opcode & 0x00FF) {
                        case 0x00E0: // CLS
                            for (int i = 0; i < 64 * 32; ++i)
                                gfx[i] = 0;
                            draw_flag = true;
                            pc += 2;
                            break;

                        case (0x00EE):  // RET
                            if (sp == 0) {
                                cerr << "! Stack underflow: RET without CALL" << endl;
                                pc += 2;
                                break;
                            }
                            sp--;                 // move stack pointer down
                            pc = stack[sp];       // pop from stack
                            cout << "RET → PC=0x" << hex << pc << endl;
                            break;
                        
                        default:
                            cout << "Unknown opcode [0x0000]: 0x" << hex << opcode << endl;
                            pc += 2;
                            break;
                    }

                case 0x1000:  // JP addr
                    nnn = opcode & 0x0FFF;
                    pc = nnn;
                    cout << "JP " << hex << nnn << endl;
                    break;

                case 0x2000:  // CALL addr
                    if (sp >= 16) {
                        cerr << "! Stack overflow: CALL" << endl;
                        break;
                    }

                    nnn = opcode & 0x0FFF;
                    stack[sp] = pc + 2;
                    sp++;
                    pc = nnn;
                    cout << "CALL 0x" << hex << nnn << endl;
                    break;

                case 0x3000:  // SE Vx, byte
                    x = (opcode & 0x0F00) >> 8;
                    nn = opcode & 0x00FF;

                    if (V[x] == nn) {
                        pc += 4;  // skip next instruction
                    } else {
                        pc += 2;
                    }

                    cout << "SE V" << hex << (int)x 
                        << ", " << setw(2) << setfill('0') << hex << (int)nn 
                        << " → " << ((V[x] == nn) ? "skip" : "no skip") << endl;
                    
                    break;
                
                case 0x4000:  // SNE Vx, byte
                    skipped = false;
                    x = (opcode & 0x0F00) >> 8;
                    nn = opcode & 0x00FF;
                    if (V[x] != nn) {
                        pc += 4;  // skip next instruction
                        skipped = true;
                    } else {
                        pc += 2;
                    }

                    cout << "SNE V" << hex << (int)x 
                        << ", " << setw(2) << setfill('0') << hex << (int)nn 
                        << " → " << (skipped ? "skip" : "no skip") << endl;
                    break;
                
                case 0x5000:  // SE Vx, Vy
                    x = (opcode & 0x0F00) >> 8;
                    y = (opcode & 0x00F0) >> 4;
                    if (V[x] == V[y]) {
                        pc += 4;  // skip next instruction
                        skipped = true;
                    } else {
                        pc += 2;
                    }

                    cout << "SE V" << hex << (int)x 
                        << ", V" << hex << (int)y 
                        << " → " << (skipped ? "skip" : "no skip") << endl;
                    break;
                
                case 0x6000:  // LD Vx, byte
                    x = (opcode & 0x0F00) >> 8;
                    nn = opcode & 0x00FF;
                    V[x] = nn;
                    pc += 2;
                    
                    cout << "LD V" << hex << (int)x 
                        << ", " << setw(2) << setfill('0') << hex << (int)nn 
                        << endl;
                    break;
                
                case 0x7000:  // ADD Vx, byte
                    x = (opcode & 0x0F00) >> 8;
                    nn = opcode & 0x00FF;

                    oldVx = V[x];
                    V[x] = (V[x] + nn) & 0xFF;
                    pc += 2;

                    cout << "ADD V" << hex << (int)x 
                        << ", " << setw(2) << setfill('0') << hex << (int)nn 
                        << " → V" << hex << (int)x << "=" << setw(2) << setfill('0') << hex << (int)V[x]
                        << " (was " << setw(2) << setfill('0') << hex << (int)oldVx << ")" 
                        << endl;
                    break;
                
                case 0x8000:
                    x = (opcode & 0x0F00) >> 8;
                    y = (opcode & 0x00F0) >> 4;
                    switch (opcode & 0x000F) {
                        case 0x0000: // LD Vx, Vy
                            V[x] = V[y];
                            pc += 2;

                            cout << "LD V" << hex << (int)x
                                << ", V" << hex << (int)y
                                << " → V" << hex << (int)x
                                << "=" << (int)V[x]
                                << " (was " << (int)oldVx << ")"
                                << endl;
                            break;
                        
                            // TODO: add 0x0001
                    }
                    
                    break;

            }
        }

};

int main() {
    Chip8 chip;
    
    return 0;
}
