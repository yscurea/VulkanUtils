#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <cassert>
#include <vector>
#include <set>
#include <optional>
#include <array>

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "VulkanInitializer.h"

namespace vulkan {
	// ------------------ instance ------------------
	VkApplicationInfo getApplicationInfo();
	VkInstanceCreateInfo getInstanceCreateInfo();




	// ----------------- swapchain -----------------
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	void selectSwapSurfaceFormat();



	namespace debug {
		extern std::vector<const char*> default_validation_layer_names = { "VK_LAYER_KHRONOS_validation" };
	}



	namespace utils {
		void createImage(
			VkDevice& device,
			uint32_t width,
			uint32_t height,
			uint32_t mipLevels,
			VkSampleCountFlagBits numSamples,
			VkFormat format,
			VkImageTiling tiling,
			VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkImage& image,
			VkDeviceMemory& imageMemory
		);

		VkImageView createImageView(
			VkDevice& device,
			VkImage image,
			VkFormat format,
			VkImageAspectFlags aspectFlags,
			uint32_t mipLevels
		);


		void createBuffer(
			VkDevice& device,
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkBuffer& buffer,
			VkDeviceMemory& bufferMemory
		);


	}
}
