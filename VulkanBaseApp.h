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
#include <string>
#include <numeric>
#include <array>

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


	void initVulkan(const char* application_name);
	virtual void prepare() {}
	void renderFrame();
	void renderLoop();
	virtual void cleanup() {}
protected:
	uint32_t width = 800;
	uint32_t height = 600;
	GLFWwindow* window;

	Instance vulkan_instance;

	Device* vulkan_device;

	Surface* vulkan_surface;

	Swapchain* vulkan_swapchain;

	VkRenderPass render_pass;
	virtual void setupRenderPass();

	VkCommandPool command_pool;
	void createCommandPool();
	std::vector<VkCommandBuffer> command_buffers;

	std::vector<VkFence> wait_fences;

	void createSynchronization();

	VkSubmitInfo submit_info;

	void presentFrame();

	void windowResize();
};
