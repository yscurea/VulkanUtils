#include "VulkanBaseApp.h"
#include "utils.h"


void VulkanBaseApp::initVulkan(const char* application_name) {
	// generate instance
	this->vulkan_instance.createInstance();

	// set debug


	// create surface
	this->vulkan_surface->createSurface(this->window);

	// select physical device
	this->vulkan_device->selectPhysicalDevice();
	this->vulkan_device->createLogicalDevice();


	// create swapchain
	// create image views
	this->vulkan_swapchain->createSwapchain();


	// create render pass
	this->setupRenderPass();




	// ----------- overrice ------------- //
	// create graphics pipeline


}





void VulkanBaseApp::renderLoop() {
	while (!glfwWindowShouldClose(window)) {
		// ƒCƒxƒ“ƒgˆ—‚ğ‚µ‚Ä
		glfwPollEvents();
		// •`‰æ‚·‚é
		renderFrame();
	}
}