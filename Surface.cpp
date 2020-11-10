#include "Surface.h"

void Surface::createSurface(VkInstance* instance, GLFWwindow* window) {
	glfwCreateWindowSurface(*instance, window, nullptr, &this->surface);
}
