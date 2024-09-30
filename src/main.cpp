#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "headers/chip8.h"

using std::cout;
using std::endl;

int main() {
	chip8* h = new chip8();
	h->setup_graphics();
	h->load_rom("C:/Users/ianga/Desktop/Codespaces/chip8/src/roms/test_opcode.ch8");

	while (!glfwWindowShouldClose(h->window)) {
		double time = glfwGetTime();
		h->emulate_cycle();

		glfwSwapBuffers(h->window);
		glfwPollEvents();
	}

	glfwDestroyWindow(h->window);
	glfwTerminate();

	return 0;
}