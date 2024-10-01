#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include <headers/chip8.h>
#include <SDL2/SDL.h>

using std::cout;
using std::endl;
using std::ios;
using std::ios_base;
using std::ifstream;
using std::string;

chip8::chip8() {
	prog_counter = 0x200;
    ind = 0;
    stack_pointer = 0;

	delay_timer = 0;
	sound_timer = 0;

	for (int i = 0; i < 4096; i++) {
		memory[i] = 0x0;
	}

	for (int i = 0; i < 2048; i++) {
		gfx[i] = 0x0;
	}

	for (int i = 0; i < 16; i++) {
		v[i] = 0x0;
		stack[i] = 0x0;
		keypad[i] = 0;
	}

	int fontset_start = 0x50;
	for (int i = 0; i < (sizeof(fontset) / sizeof(fontset[0])); i++) {
		memory[fontset_start + i] = fontset[i];
	}
}

short chip8::fetch_opcode(int addr) {
	return memory[addr] << 8 | memory[addr + 1];
}

void chip8::load_rom(const char* rom) {
	ifstream in(rom, ios_base::binary | ios_base::ate);

	if (in.is_open()) {
		std::streampos size = in.tellg();
		char* buffer = new char[size];

		in.seekg(0, ios::beg);
		in.read(buffer, size);
		in.close();

		for (int i = 0; i < size; ++i) {
			memory[512 + i] = buffer[i];
		}
	}
}

void chip8::setup_graphics() {
    SDL_Init(SDL_INIT_VIDEO);
	
	window = SDL_CreateWindow(
		"chip8", 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		640, 320, 
		SDL_WINDOW_SHOWN
	);

	renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_RenderSetLogicalSize(renderer, 640, 320);

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

	if (window == NULL) {
		cout << "Could not create window " << SDL_GetError() << endl;
		running = false;
	}
}

void chip8::update_graphics(const void* buffer, int pitch) {
	unsigned int pixels[2048];

	for (int i = 0; i < 2048; ++i) {
		unsigned int pixel = gfx[i];
		pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
		pixels[i] = (pixels[i] == 0xFF000000 ? 0xFFB7C4A3 : 0x1E211A);
	}

	SDL_UpdateTexture(texture, nullptr, pixels, 64 * sizeof(unsigned int));
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
}

bool chip8::fetch_input() {
	bool quit = false;
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT: {
				quit = true;
				break;
			} 

			case SDL_KEYDOWN: {
				switch (event.key.keysym.sym) {
					case SDLK_ESCAPE: { quit = true; break; }

					case SDLK_x: { keypad[0] = 1; break; }
					case SDLK_1: { keypad[1] = 1; break; } 
					case SDLK_2: { keypad[2] = 1; break; } 
					case SDLK_3: { keypad[3] = 1; break; } 
					case SDLK_q: { keypad[4] = 1; break; } 
					case SDLK_w: { keypad[5] = 1; break; } 
					case SDLK_e: { keypad[6] = 1; break; } 
					case SDLK_a: { keypad[7] = 1; break; } 
					case SDLK_s: { keypad[8] = 1; break; } 
					case SDLK_d: { keypad[9] = 1; break; } 
					case SDLK_z: { keypad[0xA] = 1; break; } 
					case SDLK_c: { keypad[0xB] = 1; break; } 
					case SDLK_4: { keypad[0xC] = 1; break; }
					case SDLK_r: { keypad[0xD] = 1; break; } 
					case SDLK_f: { keypad[0xE] = 1; break; } 
					case SDLK_v: { keypad[0xF] = 1; break; }
					default: { break; }
				}

				break;
			} 

			case SDL_KEYUP: {
				switch (event.key.keysym.sym) {
					case SDLK_x: { keypad[0] = 0; break; }
					case SDLK_1: { keypad[1] = 0; break; } 
					case SDLK_2: { keypad[2] = 0; break; } 
					case SDLK_3: { keypad[3] = 0; break; } 
					case SDLK_q: { keypad[4] = 0; break; } 
					case SDLK_w: { keypad[5] = 0; break; } 
					case SDLK_e: { keypad[6] = 0; break; } 
					case SDLK_a: { keypad[7] = 0; break; } 
					case SDLK_s: { keypad[8] = 0; break; } 
					case SDLK_d: { keypad[9] = 0; break; } 
					case SDLK_z: { keypad[0xA] = 0; break; } 
					case SDLK_c: { keypad[0xB] = 0; break; } 
					case SDLK_4: { keypad[0xC] = 0; break; }
					case SDLK_r: { keypad[0xD] = 0; break; } 
					case SDLK_f: { keypad[0xE] = 0; break; } 
					case SDLK_v: { keypad[0xF] = 0; break; }
					default: { break; }
				}

				break;
			}
		}	
	}

	return quit;
}

void chip8::end_graphics() {
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

    SDL_Quit();
}

void chip8::emulate_cycle() {
	short opcode = fetch_opcode(static_cast<int>(prog_counter));
	prog_counter += 2;
	draw_flag = false;

	switch (opcode & 0xF000) {
		case 0x1000: {
			prog_counter = opcode & 0x0FFF;
			break;
		}

		case 0x2000: {
			stack[stack_pointer] = prog_counter;
			++stack_pointer;
			prog_counter = opcode & 0x0FFF;
			break;
		}

		case 0x3000: {
			int x = (opcode & 0x0F00) >> 8;
			int nn = opcode & 0x00FF;

			if (v[x] == nn) {
                prog_counter += 2;
            }
            break;
		}

		case 0x4000: {	
			int x = (opcode & 0x0F00) >> 8;
			int nn = opcode & 0x00FF;

			if (v[x] != nn) {
                prog_counter += 2;
            }
            break;
		}

		case 0x5000: {
			int x = (opcode & 0x0F00) >> 8;
			int y = (opcode & 0x00F0) >> 4;

			if (v[x] == v[y]) {
				prog_counter += 2;
            }
            break;
		}

		case 0x6000:{
			int x = (opcode & 0x0F00) >> 8;
			int nn = opcode & 0x00FF;
			v[x] = nn;
			break;
		}

		case 0x7000: {
			int x = (opcode & 0x0F00) >> 8;
			int nn = opcode & 0x00FF;
			v[x] += nn;
			break;
		}

		case 0x8000: {
			int x = (opcode & 0x0F00) >> 8;
			int y = (opcode & 0x00F0) >> 4;

			switch (opcode & 0x000F) {
				case 0x0: {
					v[x] = v[y];
					break;
				}

				case 0x1: {
					v[x] = v[x] | v[y];
					break;
				}

				case 0x2: {
					v[x] = v[x] & v[y];
					break;
				}

				case 0x3: {
					v[x] = v[x] ^ v[y];
					break;
				}

				case 0x4: {
					int sum = v[x] + v[y];
					
					v[0xF] = (sum > 255 ? 1 : 0);
					v[x] = sum & 0xFF;
					break;
				}

				case 0x5: {
					v[0xF] = (v[x] > v[y] ? 1 : 0);
					v[x] -= v[y];
					break;
				}

				case 0x6: {
					v[0xF] = v[x] & 0x1u;
					v[x] >>= 1u;
					break;
				}

				case 0x7: {
					v[0xF] = (v[y] > v[x] ? 1 : 0);
					v[x] = v[y] - v[x];
					break;
				}

				case 0xE: {
					v[0xF] = (v[x] & 0x80) >> 7u;
					v[x] <<= 1;
					break;
				}
			}

			break;
		}
		
		case 0x9000: {
			int x = (opcode & 0x0F00) >> 8;
			int y = (opcode & 0x00F0) >> 4;

			if (v[x] != v[y]) {
				prog_counter += 2;
            }
            break;
		}

		case 0xA000: {
			ind = opcode & 0x0FFF;
			break;
		}
		
		case 0xB000: {
			prog_counter = v[0] + (opcode & 0x0FFF);
			break;
		}

		case 0xC000: {
			int x = (opcode & 0x0F00) >> 8;
            int nn = opcode & 0x00FF;

            v[x] = rand() % 256 & nn;
            break;
		}

		case 0xD000: {
			unsigned short x = v[(opcode & 0x0F00) >> 8] % 640;
			unsigned short y = v[(opcode & 0x00F0) >> 4] % 320;

			unsigned short spr_height = opcode & 0x000F;

			v[0xF] = 0;

			for (int height = 0; height < spr_height; ++height) {
				unsigned short spr = memory[ind + height];
				
				for (int width = 0; width < 8; ++width) {
					unsigned short spr_pix = spr & (0x80 >> width);
					unsigned short scr_pix = gfx[(y + height) * 64 + x + width];

					if (spr_pix) {
						if (scr_pix == 1) v[0xF] = 1;    

						gfx[(y + height) * 64 + x + width] ^= 1;                        
					}
				}
			}

			draw_flag = true;
			break;
		}

		case 0xE000: {
			int x = (opcode & 0x0F00) >> 8;
			int key = static_cast<int>(v[x]);

			switch (opcode & 0x00F0) {
				case 0x90: {
					if (keypad[key] == 1) prog_counter += 2;
					break;
				}

				case 0xA0: {
					if (keypad[key] == 0) prog_counter += 2;
					break;
				}
			}

			break;
		}

		case 0xF000: {
			int x = (opcode & 0x0F00) >> 8;

			switch (opcode & 0x00FF) {
				case 0x07: {
					v[x] = delay_timer;
					break;
				}

				case 0x0A: {
					if (keypad[0] == 1) { v[x] = 0; }
					else if (keypad[1] == 1) { v[x] = 1; }
					else if (keypad[2] == 1) { v[x] = 2; }
					else if (keypad[3] == 1) { v[x] = 3; }
					else if (keypad[4] == 1) { v[x] = 4; }
					else if (keypad[5] == 1) { v[x] = 5; }
					else if (keypad[6] == 1) { v[x] = 6; }
					else if (keypad[7] == 1) { v[x] = 7; }
					else if (keypad[8] == 1) { v[x] = 8; }
					else if (keypad[9] == 1) { v[x] = 9; }
					else if (keypad[10] == 1) { v[x] = 10; }
					else if (keypad[11] == 1) { v[x] = 11; }
					else if (keypad[12] == 1) { v[x] = 12; }
					else if (keypad[13] == 1) { v[x] = 13; }
					else if (keypad[14] == 1) { v[x] = 14; }
					else if (keypad[15] == 1) { v[x] = 15; }
					else { prog_counter -= 2; }
					
					break;
				}

				case 0x15: {
					delay_timer = v[x];
                    break;
				}

				case 0x18: {
                    sound_timer = v[x];
                    break;
                }

				case 0x1E: {
                    ind += v[x];
					break;
				}

				case 0x29: {
					char digit = v[x];
					ind = 0x50 + (5 * digit);

					break;
				}

				case 0x33: {
					int vx = v[x];
					memory[ind + 2] = vx % 10;
					vx /= 10;

					memory[ind + 1] = vx % 10;
                    vx /= 10;

					memory[ind] = vx;
					break;
				}

				case 0x55: {
					for (int i = 0; i <= x; ++i) {
						memory[ind + i] = v[i];
					}
					break;
				}

				case 0x65: {
					for (int i = 0; i <= x; ++i) {
						v[i] = memory[ind + i];
					}
					break;
				}
			}
			
			break;
		}

		default:
			break;
	}

	if (opcode == 0x00E0) {
		for (int i = 0; i < 2048; i++) {
			gfx[i] = 0x0;
		}
		
		draw_flag = true;

	} else if (opcode == 0x00EE) {
		--stack_pointer;
		prog_counter = stack[stack_pointer];

	} else if (opcode & 0xF000 == 0x0000) {
		ind = opcode & 0x0FFF;

	}

	if (opcode == 0x0000) {
		cout << "broken opcode found; halting" << endl;
		running = false;
	}

	if (delay_timer > 0) delay_timer--;
	
	if (sound_timer > 0) {
		if (sound_timer == 1) {
			cout << "Beep" << endl;
			sound_timer--;
		}
	}
}