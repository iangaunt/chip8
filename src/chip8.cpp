#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include <headers/chip8.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

using std::byte;
using std::cout;
using std::endl;
using std::ios;
using std::ifstream;
using std::string;
using std::vector;

chip8::chip8() {
	prog_counter = 0x200;
    opcode = 0;
    ind = 0;
    stack_pointer = 0;

	delay_timer = 0;
	sound_timer = 0;

	int fontset_start = 0x50;
	for (int i = 0; i < (sizeof(fontset) / sizeof(fontset[0])); i++) {
		memory[fontset_start + i] = fontset[i];
	}
}

short chip8::fetch_opcode(int addr) {
	return memory[addr] << 8 | memory[addr + 1];
}

void chip8::load_rom(const char* rom) {
	ifstream in(rom, ios::binary);

	in.seekg(0, ios::end);
	auto length = in.tellg();
	in.seekg(0, ios::beg);

	vector<byte> buffer(length);
	in.read(reinterpret_cast<char*>(buffer.data()), length);

    in.close();

	for (int i = 0; i < buffer.size(); i++) {
		memory[512 + i] = static_cast<unsigned char>(buffer.at(i));
	}
}

void chip8::print_board() {
	for (int i = 0; i < 32; i++) {
		string row = "";
		for (int j = 0; j < 64; j++) {
			if (gfx[i * 32 + j] == 0xFF) {
                row += "██";
            } else {
                row += "  ";
            }
		}
	}
}

void chip8::setup_graphics() {
    glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(64 * chip8::window_size_modifier, 32 * chip8::window_size_modifier, "chip8", NULL, NULL);

	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);
	gladLoadGL();
	glViewport(0, 0, 64 * chip8::window_size_modifier, 32 * chip8::window_size_modifier);
}

void chip8::emulate_cycle() {
	short opcode = fetch_opcode(static_cast<int>(prog_counter));
	prog_counter += 2;

	cout << "0x" << std::hex << std::uppercase << opcode << endl;

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
					char orig_vx = v[x];
					v[x] += v[y];

					if (orig_vx > v[x]) {
						v[15] = 1;
					}
					break;
				}

				case 0x5: {
					char orig_vx = v[x];
					v[x] -= v[y];

					if (orig_vx < v[x]) {
						v[15] = 1;
					}
					break;
				}

				case 0x7: {
					char orig_vx = v[x];
					v[x] = v[y] - v[x];
					
					if (orig_vx < v[x]) {
						v[15] = 0;
					}
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
			prog_counter = opcode & 0x0FFF;
			break;
		}
		
		case 0xB000: {
			prog_counter = opcode & 0x0FFF + v[0];
			break;
		}

		case 0xC000: {
			int x = (opcode & 0x0F00) >> 8;
            int nn = opcode & 0x00FF;

            v[x] = rand() % 256 & nn;
            break;
		}

		case 0xD000: {
			int x = (opcode & 0x0F00) >> 8;
			int y = (opcode & 0x00F0) >> 4;
			int height = opcode & 0x000F;

			int indc = ind;

			for (int i = 0; i < 8; i++) {
				for (int j = 0; j < height; j++) {
					gfx[(y + height) * 32 + (x + i)] = memory[indc];
					indc++;
				}
			}

			break;
		}

		case 0xE000: {
			switch (opcode & 0x00F0 >> 4) {
				case 0x9: {
					int x = (opcode & 0x0F00) >> 8;
					if (keypad[v[x]] == 1) {
						prog_counter += 2;
					}

					break;
				}

				case 0xA: {
					int x = (opcode & 0x0F00) >> 8;
					if (keypad[v[x]] == 0) {
						prog_counter += 2;
					}

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
					bool keyPressed = false;
					for (int i = 0; i < 16; i++) {
						if (keypad[i]) {
							keyPressed = true;
							keypad[x] = i;
						}
					}

					if (!keyPressed) prog_counter -= 2;

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
					for (int i = 0; i <= x; i++) {
						memory[ind + i] = v[i];
					}
					ind += x + 1;
					break;
				}

				case 0x65: {
					for (int i = 0; i <= x; i++) {
						v[i] = memory[ind + i];
					}
					ind += x + 1;
					break;
				}
			}
			
			break;
		}

		default:
			break;
	}

	if (opcode == 0x00E0) {
		memset(gfx, 0, sizeof(gfx));

	} else if (opcode == 0x00EE) {
		--stack_pointer;
		prog_counter = stack[stack_pointer];

	} else if (opcode & 0xF000 == 0x0000) {
		ind = opcode & 0x0FFF;
		
	}

	if (delay_timer > 0) delay_timer--;
	
	if (sound_timer > 0) {
		if (sound_timer == 1) {
			cout << "Beep" << endl;
			sound_timer--;
		}
	}
}