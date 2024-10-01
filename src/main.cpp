#include <chrono>
#include <iostream>
#include <SDL2/SDL.h>

#include "headers/chip8.h"

using std::cout;
using std::endl;

int main(int argv, char** args) {
	chip8* c8 = new chip8();
	c8->setup_graphics();
	c8->load_rom("C:/Users/ianga/Desktop/Codespaces/chip8/src/roms/test_opcode.ch8");

	bool quit = false;
	int count = 0;
	
	auto cycle_delay = 2;
	auto last_cycle = std::chrono::high_resolution_clock::now();

	while (c8->running && !quit) {
		quit = c8->fetch_input();

		auto current = std::chrono::high_resolution_clock::now();
		float delay = std::chrono::duration<float, std::chrono::milliseconds::period>(current - last_cycle).count();

		if (delay > cycle_delay) {
			c8->emulate_cycle();

			if (c8->draw_flag) {
				c8->update_graphics(c8->gfx, sizeof(c8->gfx[0]) * 64);
			}
			last_cycle = current;
		}
	}

	c8->end_graphics();

	return 0;
}