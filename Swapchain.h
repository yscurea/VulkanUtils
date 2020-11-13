#pragma once

#include "Platform.h"
#include "utils.h"

class Swapchain
{
private:
	VkInstance* instance;
	VkDevice* device;
	VkPhysicalDevice* physical_device;
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

	SwapChainSupportDetails querySwapChainSupport();
public:
	VkFormat swapchain_image_format;
	VkColorSpaceKHR color_space;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	uint32_t image_count;
	std::vector<VkImage> swapchain_images;
	uint32_t queue_node_index = UINT32_MAX;
	VkExtent2D swapchain_extent;

	void createSwapchain(uint32_t* width, uint32_t* height);
	uint32_t acquireNextImage(VkSemaphore presentCompleteSemaphore);
	VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
	void cleanup();
};
