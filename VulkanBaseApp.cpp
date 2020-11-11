#include "VulkanBaseApp.h"
#include "utils.h"


void VulkanBaseApp::initVulkan(const char* application_name) {
	// instance
	this->vulkan_instance.createInstance();

	this->vulkan_surface = new Surface();
	this->vulkan_surface->createSurface(&this->vulkan_instance, this->window);

	// device
	this->vulkan_device = new Device(this->vulkan_instance.getInstance());
	this->vulkan_device->selectPhysicalDevice(nullptr);
	this->vulkan_device->createLogicalDevice();

	// swapchain
	this->vulkan_swapchain = new Swapchain();
	this->vulkan_swapchain->createSwapchain();


	// renderpass
}


void VulkanBaseApp::run() {
	this->initVulkan("name");

	this->prepare();

	while (glfwWindowShouldClose(this->window) == false) {
		glfwPollEvents();

		this->render();
	}

	vkDeviceWaitIdle(*this->vulkan_device->getLogicalDevice());

	this->cleanup();
}
