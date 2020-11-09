#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <array>
#include <numeric>
#include <ctime>
#include <iostream>
#include <chrono>
#include <random>
#include <algorithm>
#include <sys/stat.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <numeric>
#include <array>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Instance.h"
#include "Device.h"
#include "Surface.h"
#include "Swapchain.h"


class VulkanBaseApp {
public:
	// constructor
	VulkanBaseApp();
	virtual ~VulkanBaseApp();

	void run();

	// use with override in inheritance class
	virtual void makeCommand(VkCommandBuffer command_buffer) {}

protected:
	Instance vulkan_instance;
	Device* vulkan_device;
	Swapchain* vulkan_swapchain;

	void initVulkan(const char* application_name);
	void render();
};
