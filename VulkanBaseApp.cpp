#include "VulkanBaseApp.h"

#include <vk_mem_alloc.h>

// public ---------------------
void VulkanBaseApp::initVulkan() {
	// generate instance
	this->createInstance();

#ifdef _DEBUG
	void setupDebugMessenger();
#endif

	// create surface
	this->createSurface();

	// select physical device
	this->selectPhysicalDevice();

	// create logical device
	this->createLogicalDevice();

	// create swapchain
	this->createSwapchain();



	// create render pass
	this->setupRenderPass();

	// create command pool
	this->createCommandPool();

	// create command buffers
	this->createCommandBuffers();
}

void VulkanBaseApp::renderFrame() {
	// ‘Ò‚¿
	vkWaitForFences(this->device, 1, &this->in_flight_fences[this->current_frame], VK_TRUE, UINT64_MAX);

	// ŽŸ‚Ì‰æ‘œ‚ðŽæ“¾
	uint32_t image_index;
	VkResult result =
		vkAcquireNextImageKHR(
			this->device,
			this->swapchain,
			UINT64_MAX,
			this->image_available_semaphores[this->current_frame],
			VK_NULL_HANDLE,
			&image_index
		);

	this->render();

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		this->recreateSwapchain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}
}

void VulkanBaseApp::renderLoop() {
	while (!glfwWindowShouldClose(this->window)) {
		glfwPollEvents();
		this->renderFrame();
	}
}


void VulkanBaseApp::submitFrame(uint32_t& image_index, VkSemaphore* render_finished_semaphores) {
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = render_finished_semaphores;

	VkSwapchainKHR swapchains[] = { this->swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;

	presentInfo.pImageIndices = &image_index;

	VkResult result = vkQueuePresentKHR(this->present_queue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->framebuffer_resized) {
		this->framebuffer_resized = false;
		this->recreateSwapchain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}
}

void VulkanBaseApp::cleanup() {

	this->deleteInstance();
}

// --------------------- instance ----------------------
void VulkanBaseApp::createInstance() {
#ifdef _DEBUG
	if (!checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}
#endif

	VkApplicationInfo app_info = vk::utils::getApplicationInfo();

	VkInstanceCreateInfo createInfo = vk::utils::getInstanceCreateInfo();
	createInfo.pApplicationInfo = &app_info;

	std::vector<const char*> extensions;
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> tmp_extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef _DEBUG
		tmp_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		extensions = tmp_extensions;
	}

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef _DEBUG
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	createInfo.enabledLayerCount = static_cast<uint32_t>(validation_layer_names.size());
	createInfo.ppEnabledLayerNames = validation_layer_names.data();

	populateDebugMessengerCreateInfo(debugCreateInfo);
	createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#endif

	createInfo.enabledLayerCount = 0;
	createInfo.pNext = nullptr;

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

// -------------------- surface ------------------------
void VulkanBaseApp::createSurface() {
	// #ifdef USE_GLFW
	if (glfwCreateWindowSurface(this->instance, this->window, nullptr, &this->surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface");
	}
	// #elseif
	// #endif
}


// -------------------- swapchain ----------------------
void VulkanBaseApp::createSwapchain() {
	vulkan::SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

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

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
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

	if (vkCreateSwapchainKHR(this->device, &createInfo, nullptr, &this->swapchain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(this->device, this->swapchain, &imageCount, nullptr);
	swapchain_images.resize(imageCount);
	vkGetSwapchainImagesKHR(this->device, this->swapchain, &imageCount, swapchain_images.data());

	swapchain_image_format = surfaceFormat.format;
	swapchain_extent = extent;

	// ---------- create image views -------------
	this->swapchain_image_views.resize(swapchain_images.size());

	for (uint32_t i = 0; i < swapchain_images.size(); i++) {
		this->swapchain_image_views[i] = vulkan::utils::createImageView(swapchain_images[i], swapchain_image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

}
void VulkanBaseApp::recreateSwapchain() {

}
void VulkanBaseApp::createColorResources() {
	VkFormat colorFormat = swapchain_image_format;

	vulkan::utils::createImage(this->device, this->swapchain_extent.width, this->swapchain_extent.height, 1, this->sample_count_flag_bits, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->color_image, this->color_image_memory);
	this->color_image_view = vulkan::utils::createImageView(this->device, this->color_image, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}
void VulkanBaseApp::createDepthResources() {
	VkFormat depthFormat = this->findDepthFormat();

	vulkan::utils::createImage(this->device, this->swapchain_extent.width, this->swapchain_extent.height, 1, this->msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->depth_image, this->depth_image_memory);
	this->depth_image_view = vulkan::utils::createImageView(this->device, this->depth_image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}


// --------------------- render pass -------------------
void VulkanBaseApp::setupRenderPass() {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = this->swapchain_image_format;
	colorAttachment.samples = this->sample_count_flag_bits;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	VkFormat format;
	auto candidates = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	auto tiling = VK_IMAGE_TILING_OPTIMAL;
	auto features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
	{
		bool is_fulled = false;
		for (VkFormat f : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);
			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				format = f;
				is_fulled = true;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				format = f;
				is_fulled = true;
			}
		}
		if (is_fulled == false)
			throw std::runtime_error("failed to find supported format!");
	}
	depthAttachment.format = format;
	depthAttachment.samples = this->sample_count_flag_bits;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = this->swapchain_image_format;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// color buffer
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// depth buffer
	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// MSAA
	VkAttachmentReference colorAttachmentResolveRef{};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = &colorAttachmentResolveRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(this->device, &renderPassInfo, nullptr, &this->render_pass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}


// ---------------- command --------------
void VulkanBaseApp::createCommandPool() {
	VkCommandPoolCreateInfo cmd_pool_info = {};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.queueFamilyIndex = this->graphics_queue_index.value();
	cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (vkCreateCommandPool(device, &cmd_pool_info, nullptr, &this->command_pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool");
	}
}

void VulkanBaseApp::createCommandBuffers() {
	this->command_buffers.resize(this->swapchain_image_count);

	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = this->command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = (uint32_t)this->command_buffers.size();

	if (vkAllocateCommandBuffers(this->device, &alloc_info, this->command_buffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}
