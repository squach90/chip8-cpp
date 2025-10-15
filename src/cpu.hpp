#include <fstream>
#include <cstdint>
#include <string>
#include <iostream>
#include <iomanip>
#include <random>
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

            std::cout << "Program loaded, first 4 bytes: "
                << std::hex
                << (int)memory[0x200] << " "
                << (int)memory[0x201] << " "
                << (int)memory[0x202] << " "
                << (int)memory[0x203] << std::dec
                << std::endl;


            delete[] buffer;
            file.close();
        }


        void emulation_cycle() {
            // Fetch
            uint16_t opcode = (memory[pc] << 8) | memory[pc + 1];
            uint16_t nnn;
            uint8_t x;
            uint8_t y;
            uint8_t nn;
            uint8_t oldVx;
            uint16_t result;
            uint8_t height;
            uint8_t yline;
            uint8_t xline;
            uint8_t pixel;
            uint16_t index;
            uint16_t running;

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

                        case 0x00EE:  // RET
                            if (sp == 0) {
                                cerr << "! Stack underflow: RET without CALL" << endl;
                                running = false;
                                return;
                            }
                            sp--;
                            pc = stack[sp];
                            if (pc < 0x200 || pc > 0xFFF) {
                                cerr << "! Invalid return address: 0x" << hex << pc << endl;
                                running = false;
                                return;
                            }
                            cout << "RET → PC=0x" << hex << pc << endl;
                            break;

                        default:
                            cerr << "! Unknown opcode [0x0000]: 0x" << hex << opcode << endl;
                            running = false;
                            return;
                    }
                    break;

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
                        case 0x0001:  // OR Vx, Vy
                            V[x] |= V[y];
                            pc += 2;
                            cout << "OR V" << hex << uppercase << (int)x 
                                << ", V" << (int)y 
                                << " → V" << (int)x << "=" 
                                << setw(2) << setfill('0') << (int)V[x]
                                << nouppercase << dec << endl;
                            break;
                        
                        case 0x0002:  // AND Vx, Vy
                            V[x] &= V[y];
                            pc += 2;
                            cout << "AND V" << hex << uppercase << (int)x
                                << ", V" << (int)y
                                << " → V" << (int)x << "="
                                << setw(2) << setfill('0') << (int)V[x]
                                << nouppercase << dec << endl;
                            break;
                        
                        case 0x0003:  // XOR Vx, Vy
                            V[x] ^= V[y];
                            pc += 2;
                            cout << "XOR V" << hex << uppercase << (int)x
                                << ", V" << (int)y
                                << " → V" << (int)x << "="
                                << setw(2) << setfill('0') << (int)V[x]
                                << nouppercase << dec << endl;
                            break;
                        
                        case 0x0004:  // ADD Vx, Vy
                            result = V[x] + V[y];
                            V[0xF] = (result > 0xFF) ? 1 : 0;
                            V[x] = result & 0xFF;
                            pc += 2;

                            cout << "ADD V" << hex << uppercase << (int)x
                                << ", V" << (int)y
                                << " → V" << (int)x << "="
                                << setw(2) << setfill('0') << (int)V[x]
                                << " VF=" << (int)V[0xF]
                                << nouppercase << dec << endl;
                            break;
                        
                        case 0x0005:  // SUB Vx, Vy
                            V[0xF] = (V[x] >= V[y]) ? 1 : 0;
                            V[x] = (V[x] - V[y]) & 0xFF;
                            pc += 2;
                            cout << "SUB V" << hex << uppercase << (int)x
                                << ", V" << (int)y
                                << " → V" << (int)x << "="
                                << setw(2) << setfill('0') << (int)V[x]
                                << ", VF=" << (int)V[0xF]
                                << nouppercase << dec << endl;
                            break;
                        
                        case 0x0006:  // SHR Vx {, Vy}
                            V[0xF] = V[x] & 0x1;
                            V[x] >>= 1;
                            pc += 2;
                            break;
                        
                        case 0x0007:  // SUBN Vx, Vy
                            V[0xF] = (V[y] >= V[x]) ? 1 : 0;
                            V[x] = (V[y] - V[x]) & 0xFF;
                            pc += 2;
                            cout << "SUBN V" << hex << (int)x 
                                << ", V" << (int)y 
                                << " → V" << (int)x << "=" << setw(2) << setfill('0') << hex << (int)V[x]
                                << ", VF=" << (int)V[0xF]
                                << endl;
                            break;
                        
                        case 0x000E:  // SHL Vx {, Vy}
                            V[0xF] = (V[x] & 0x80) >> 7;
                            V[x] = (V[x] << 1) & 0xFF;
                            pc += 2;
                            cout << "SHL V" << hex << (int)x 
                                << " → V" << (int)x << "=" << setw(2) << setfill('0') << hex << (int)V[x]
                                << ", VF=" << (int)V[0xF]
                                << endl;
                            break;

                        default:
                            cout << "Unknown opcode [0x8000]: 0x" << hex << opcode << endl;
                            pc += 2;
                            break;
                        
                        break;
                    }

                    case 0x9000:  // SNE Vx, Vy
                        x = (opcode & 0x0F00) >> 8;
                        y = (opcode & 0x00F0) >> 4;
                        if (V[x] != V[y]) {
                            pc += 4;
                            skipped = true;
                        } else {
                            pc += 2;
                        }

                        cout << "SNE V" << hex << (int)x 
                            << ", V" << (int)y 
                            << " → " << (skipped ? "skip" : "no skip")
                            << endl;
                        break;
                    
                    case 0xA000:  // LD I, addr
                        nnn = opcode & 0x0FFF;
                        I = nnn;
                        pc += 2;
                        cout << "LD I = " << setw(3) << setfill('0') << hex << (int)nnn << endl;
                        break;
                    
                    case 0xC000: {  // RND Vx, byte
                        uint8_t x = (opcode & 0x0F00) >> 8;
                        uint8_t nn = opcode & 0x00FF;

                        static std::random_device rd;
                        static std::mt19937 gen(rd());
                        std::uniform_int_distribution<uint8_t> dis(0, 255);
                        uint8_t rnd = dis(gen);

                        V[x] = rnd & nn;

                        cout << "RND V" << hex << uppercase << (int)x 
                            << " = " << (int)rnd 
                            << " & " << setw(2) << setfill('0') << (int)nn 
                            << " → " << (int)V[x]
                            << nouppercase << dec << endl;

                        pc += 2;
                        break;
                    }


                    case 0xD000: { // DRW Vx, Vy, nibble
                        uint8_t vx = (opcode & 0x0F00) >> 8;
                        uint8_t vy = (opcode & 0x00F0) >> 4;
                        x = V[vx];
                        y = V[vy];
                        height = opcode & 0x000F;
                        V[0xF] = 0;

                        for (yline = 0; yline < height; ++yline) {
                            pixel = memory[I + yline];
                            for (xline = 0; xline < 8; ++xline) {
                                if (pixel & (0x80 >> xline)) {
                                    int xpos = x + xline;
                                    int ypos = y + yline;
                                    
                                    if (xpos >= 64 || ypos >= 32) continue;

                                    int index = xpos + ypos * 64;
                                    if (gfx[index] == 1) {
                                        V[0xF] = 1;
                                    }
                                    gfx[index] ^= 1;
                                }
                            }
                        }

                        draw_flag = true;
                        pc += 2;

                        break;
                    }



                    
                    break;

                    case 0xE000:
                        x = (opcode & 0x0F00) >> 8;
                        switch (opcode & 0x00FF) {
                            case 0x009E:  // SKP Vx
                                if (key[V[x]]) {
                                    pc += 4;
                                } else {
                                    pc += 2;
                                }

                                cout << "SKP V" << hex << uppercase << (int)x
                                    << " (key=" << setw(1) << setfill('0') << hex << (int)V[x] << ") → "
                                    << ((key[V[x]]) ? "skip" : "no skip")
                                    << endl;

                                break;
                            
                            case 0x00A1:  // SKNP Vx
                                if (!key[V[x]]) {
                                    pc += 4;
                                } else {
                                    pc += 2;
                                }
                                cout << "SKNP V" << hex << uppercase << (int)x 
                                    << " (key=" << setw(1) << setfill('0') << hex << (int)V[x] << ") → "
                                    << ((!key[V[x]]) ? "skip" : "no skip") 
                                    << endl;
                                break;

                            
                            default:
                                cout << "Unknown opcode [0xE000]: 0x" << hex << opcode << endl;
                                pc += 2;
                                break;

                        }
                    break;

                    case 0xF000:
                        x = (opcode & 0x0F00) >> 8;
                        switch (opcode & 0x00FF) {
                            case 0x0007: { // LD Vx, DT
                                V[x] = delay_timer;
                                cout << "LD V" << hex << uppercase << (int)x 
                                    << ", DT (" << dec << (int)delay_timer << ")" 
                                    << endl;
                                pc += 2;
                                break;
                            }

                            case 0x000A: { // LD Vx, K (wait for key)
                                int key_pressed = -1;
                                for (int i = 0; i < 16; ++i) {
                                    if (key[i] != 0) {
                                        V[x] = i;
                                        key_pressed = i;
                                        break;
                                    }
                                }
                                if (key_pressed == -1) {
                                    

                                    return; // ou pc += 2; si tu veux avancer même sans entrée
                                } else {
                                    cout << "LD V" << hex << uppercase << (int)x
                                        << ", K → key " << hex << uppercase << key_pressed
                                        << endl;
                                    pc += 2;
                                }
                                break;
                            }


                            
                            case 0x0015:  // LD DT, Vx
                                delay_timer = V[x];
                                cout << "LD DT, V" << hex << uppercase << (int)x 
                                    << " (" << dec << (int)V[x] << ")" << endl;
                                pc += 2;
                                break;

                            case 0x0018: // LD ST, Vx
                                sound_timer = V[x];
                                cout << "LD ST, V" << hex << uppercase << (int)x 
                                    << " (" << dec << (int)V[x] << ") → sound_timer=" 
                                    << dec << (int)sound_timer << endl;
                                pc += 2;
                                break;
                    

                            case 0x001E: // ADD I, Vx
                                I += V[x];
                                cout << "ADD I, V" << hex << uppercase << (int)x 
                                    << " → I=" << setw(3) << setfill('0') << hex << uppercase << (int)I 
                                    << endl;
                                pc += 2;
                                break;
                            

                            case 0x0029: // LD F, Vx
                                I = 0x50 + (V[x] * 5);
                                cout << "LD F, V" << hex << uppercase << (int)x 
                                    << " → I=" << setw(3) << setfill('0') << hex << uppercase << (int)I 
                                    << endl;
                                pc += 2;
                                break;
                            

                            case 0x0033: { // LD B, Vx (BCD)
                                uint8_t value = V[x];
                                memory[I]     = value / 100;
                                memory[I + 1] = (value / 10) % 10;
                                memory[I + 2] = value % 10;
                                cout << "LD B, V" << hex << uppercase << (int)x 
                                    << " (" << dec << (int)value << ") → [" 
                                    << setw(3) << setfill('0') << hex << (int)I << "]" 
                                    << endl;
                                pc += 2;
                                break;
                            }

                            case 0x0055: // LD [I], Vx
                                for (int i = 0; i <= x; ++i)
                                    memory[I + i] = V[i];
                                cout << "LD [I], V" << hex << uppercase << (int)x << endl;
                                pc += 2;
                                break;
                            

                            case 0x0065: // LD Vx, [I]
                                for (int i = 0; i <= x; ++i)
                                    V[i] = memory[I + i];
                                cout << "LD V" << hex << uppercase << (int)x << ", [I]" << endl;
                                pc += 2;
                                break;
                            

                            default:
                                cout << "Unknown opcode [0xF000]: 0x" 
                                    << setw(4) << setfill('0') << hex << uppercase << (int)opcode 
                                    << endl;
                                pc += 2;
                                break;
                            
                        }
                        break;
                    

            }

            
        }
        
        void update_timers() {
            if (delay_timer > 0)
                delay_timer--;
            if (sound_timer > 0)
                sound_timer--;
        }
};