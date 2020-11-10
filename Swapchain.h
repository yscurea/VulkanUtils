#pragma once

#include "Platform.h"
#include "utils.h"

typedef struct SwapChainImageBuffers {
	VkImage image;
	VkImageView view;
} SwapChainImageBuffer;

class Swapchain
{
private:
	VkInstance* instance;
	VkDevice* device;
	VkPhysicalDevice* physicalDevice;
	VkSurfaceKHR* surface;
	// Function pointers
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR fpQueuePresentKHR;
public:
	VkFormat color_format;
	VkColorSpaceKHR color_space;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	uint32_t image_count;
	std::vector<vk::utils::Image> swapchain_images;
	uint32_t queue_node_index = UINT32_MAX;

	void createSwapchain(uint32_t* width, uint32_t* height, bool vsync = false);
	uint32_t acquireNextImage(VkSemaphore presentCompleteSemaphore);
	VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
	void cleanup();

};
