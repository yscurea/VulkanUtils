#pragma once

#include <vulkan/vulkan.h>

namespace vk::utils::initializer {

	inline VkApplicationInfo getAppInfo() {
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "SampleName";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;
		return appInfo;
	}
	inline VkCommandPoolCreateInfo getCommandPoolCreateInfo() {
		VkCommandPoolCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		return ci;
	}
}
