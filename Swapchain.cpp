#include "Swapchain.h"


// in this case, use glfw window

vk::utils::SwapChainSupportDetails Swapchain::querySwapChainSupport() {
	vk::utils::SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*this->physical_device, *this->surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(*this->physical_device, *this->surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(*this->physical_device, *this->surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(*this->physical_device, *this->surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(*this->physical_device, *this->surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}





void Swapchain::createSwapchain(uint32_t* width, uint32_t* height) {

	SwapChainSupportDetails swapChainSupport = querySwapChainSupport();

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physical_device);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(*device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(*device, swapchain, &imageCount, nullptr);
	this->swapchain_images.resize(imageCount);
	vkGetSwapchainImagesKHR(*device, swapchain, &imageCount, this->swapchain_images.data());

	this->swapchain_image_format = surfaceFormat.format;
	this->swapchain_extent = extent;



	// color resource
	VkFormat colorFormat = this->swapchain_image_format;

	vk::utils::createImage(this->swapchain_extent.width, this->swapchain_extent.height, 1, this->sample_count_flag_bits, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->color_image, this->color_image_memory);
	this->colorImageView = createImageView(this->colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);



	// depth resource
	VkFormat depthFormat = findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);

	this->createImage(this->swapChainExtent.width, this->swapChainExtent.height, 1, this->msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->depthImage, this->depthImageMemory);
	this->depth_image_view = this->createImageView(this->depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);




	// frame buffer
	this->swapChainFramebuffers.resize(this->swapChainImageViews.size());

	for (size_t i = 0; i < this->swapChainImageViews.size(); i++) {
		std::array<VkImageView, 3> attachments = {
			this->colorImageView,
			this->depthImageView,
			this->swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = render_pass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = this->swapchain_extent.width;
		framebufferInfo.height = this->swapchain_extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(*this->device, &framebufferInfo, nullptr, &this->swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}
