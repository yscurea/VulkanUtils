#include "VulkanBaseApp.h"
#include "utils.h"


void VulkanBaseApp::initVulkan(const char* application_name) {
	this->vulkan_instance.createInstance();
	this->vulkan_device;
}
