#pragma once

#include <vulkan/vulkan.h>

class Texture
{
	VkDevice* device;
	VkDeviceMemory* device_memory = VK_NULL_HANDLE;
	VkImage image;
	VkImageView view;
	uint32_t width;
	uint32_t height;
	uint32_t mip_level;
	VkDescriptorImageInfo descriptor_image_info;
	VkSampler sampler;
};

