#ifndef CHIP8_H
#define CHIP8_H

#include <SDL2/SDL.h>
class chip8 {
    public: 
        bool running = true;
        bool draw_flag = false;
        
        unsigned short opcode;
        unsigned char memory[4096]; 
        unsigned char v[16]; // Registers (V0 - VE)

        unsigned short ind;
        unsigned short prog_counter;

        unsigned int gfx[64 * 32];

        unsigned char delay_timer;
        unsigned char sound_timer;

        unsigned short stack[16];
        unsigned short stack_pointer;

        unsigned int keypad[16];

        unsigned char fontset[80] = {
            0xF0, 0x90, 0x90, 0x90, 0xF0, 
            0x20, 0x60, 0x20, 0x20, 0x70, 
            0xF0, 0x10, 0xF0, 0x80, 0xF0, 
            0xF0, 0x10, 0xF0, 0x10, 0xF0, 
            0x90, 0x90, 0xF0, 0x10, 0x10, 
            0xF0, 0x80, 0xF0, 0x10, 0xF0, 
            0xF0, 0x80, 0xF0, 0x90, 0xF0, 
            0xF0, 0x10, 0x20, 0x40, 0x40, 
            0xF0, 0x90, 0xF0, 0x90, 0xF0, 
            0xF0, 0x90, 0xF0, 0x10, 0xF0, 
            0xF0, 0x90, 0xF0, 0x90, 0x90, 
            0xE0, 0x90, 0xE0, 0x90, 0xE0, 
            0xF0, 0x80, 0x80, 0x80, 0xF0, 
            0xE0, 0x90, 0x90, 0x90, 0xE0, 
            0xF0, 0x80, 0xF0, 0x80, 0xF0, 
            0xF0, 0x80, 0xF0, 0x80, 0x80  
        };

        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Texture* texture;

        int window_size_modifier = 10;

        chip8();

        short fetch_opcode(int addr);
        
        void load_rom(const char* filename);
        void setup_graphics();

        void emulate_cycle();
        void update_graphics(void const* buffer, int pitch);
        bool fetch_input();

        void end_graphics();

};

#endif