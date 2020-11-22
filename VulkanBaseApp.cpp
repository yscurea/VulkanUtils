#include "VulkanBaseApp.h"

#include <vk_mem_alloc.h>


// ------------ static -------------
VkFormat findSupportedFormat(VkPhysicalDevice& physical_device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}
	throw std::runtime_error("failed to find supported format!");
}

VkFormat findDepthFormat(VkPhysicalDevice physical_device) {
	return findSupportedFormat(
		physical_device,
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}



// public ---------------------
void VulkanBaseApp::initVulkan() {
	// インスタンス作成
	this->createInstance();

#ifdef _DEBUG
	void setupDebugMessenger();
#endif

	// サーフェス作成、todo :プラットフォームによる条件コンパイルする
	this->createSurface();

	// GPUの選択
	this->selectPhysicalDevice();

	// 論理デバイス作成
	this->createLogicalDevice();

	// スワップチェイン作成、virtualのほうがいい？
	this->createSwapchain();



	// レンダーパス作成、継承先で作成方法は変更
	this->setupRenderPass();

	// コマンドプール作成
	this->createCommandPool();

	// コマンドバッファ作成、コマンドは積まない
	this->createCommandBuffers();
}

void VulkanBaseApp::renderFrame() {
	// 待ち
	vkWaitForFences(this->device, 1, &this->in_flight_fences[this->current_frame], VK_TRUE, UINT64_MAX);

	// 次の画像index取得
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

	// 描画、継承先でオーバーライドする
	this->updateFrame();

	// コマンド送信
	this->submitFrame(image_index);
	// 表示
	this->presentFrame(image_index, &this->render_finished_semaphores[this->current_frame]);
}

void VulkanBaseApp::renderLoop() {
	while (!glfwWindowShouldClose(this->window)) {
		glfwPollEvents();
		this->renderFrame();
	}
}

void VulkanBaseApp::submitFrame(uint32_t image_index) {
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = { this->image_available_semaphores[current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &this->command_buffers[image_index];

	VkSemaphore signal_semaphores[] = { this->render_finished_semaphores[current_frame] };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	vkResetFences(this->device, 1, &this->in_flight_fences[current_frame]);

	if (vkQueueSubmit(this->graphics_queue, 1, &submit_info, this->in_flight_fences[current_frame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}
}

void VulkanBaseApp::presentFrame(uint32_t image_index, VkSemaphore* render_finished_semaphores) {
	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = render_finished_semaphores;

	VkSwapchainKHR swapchains[] = { this->swapchain };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchains;

	present_info.pImageIndices = &image_index;

	VkResult result = vkQueuePresentKHR(this->present_queue, &present_info);

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

	VkApplicationInfo app_info = vulkan::getApplicationInfo();

	VkInstanceCreateInfo createInfo = vulkan::getInstanceCreateInfo();
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
	vulkan::SwapChainSupportDetails swapchainSupport = vulkan::querySwapChainSupport(this->physical_device, this->surface);

	VkSurfaceFormatKHR surfaceFormat = vulkan::chooseSwapSurfaceFormat(swapchainSupport.formats);
	VkPresentModeKHR presentMode = vulkan::chooseSwapPresentMode(swapchainSupport.presentModes);
	VkExtent2D extent = vulkan::chooseSwapExtent(swapchainSupport.capabilities, this->window);

	uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
	if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) {
		imageCount = swapchainSupport.capabilities.maxImageCount;
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

	// queue indexを更新する
	// QueueFamilyIndices indices = findQueueFamilies(this->physical_device);
	uint32_t queueFamilyIndices[] = { this->graphics_queue_index.value(), this->present_queue_index.value() };

	if (this->graphics_queue_index != this->present_queue_index) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(this->device, &createInfo, nullptr, &this->swapchain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(this->device, this->swapchain, &imageCount, nullptr);
	this->swapchain_images.resize(imageCount);
	vkGetSwapchainImagesKHR(this->device, this->swapchain, &imageCount, this->swapchain_images.data());

	this->swapchain_image_format = surfaceFormat.format;
	this->swapchain_extent = extent;

	// ---------- create image views -------------
	this->swapchain_image_views.resize(swapchain_images.size());

	for (uint32_t i = 0; i < swapchain_images.size(); i++) {
		this->swapchain_image_views[i] = vulkan::utils::createImageView(this->device, this->swapchain_images[i], this->swapchain_image_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	this->createColorResources();
	this->createDepthResources();

	this->createSyncObjects();

}
void VulkanBaseApp::recreateSwapchain() {
	int width = 0, height = 0;
	glfwGetFramebufferSize(this->window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(this->window, &width, &height);
		glfwWaitEvents();
	}

	// 待ち
	vkDeviceWaitIdle(this->device);

	this->deleteSwapchain();

	this->createSwapchain();

	createColorResources();
	createDepthResources();
}
void VulkanBaseApp::createColorResources() {
	VkFormat colorFormat = swapchain_image_format;

	vulkan::utils::createImage(this->device, this->physical_device, this->swapchain_extent.width, this->swapchain_extent.height, 1, this->sample_count_flag_bits, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->color_image, this->color_image_memory);
	this->color_image_view = vulkan::utils::createImageView(this->device, this->color_image, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}
void VulkanBaseApp::createDepthResources() {
	VkFormat depthFormat = findDepthFormat(this->physical_device);

	vulkan::utils::createImage(this->device, this->physical_device, this->swapchain_extent.width, this->swapchain_extent.height, 1, this->sample_count_flag_bits, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->depth_image, this->depth_image_memory);
	this->depth_image_view = vulkan::utils::createImageView(this->device, this->depth_image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}
void VulkanBaseApp::createSyncObjects() {
	this->image_available_semaphores.resize(semaphores_size);
	this->render_finished_semaphores.resize(semaphores_size);
	this->in_flight_fences.resize(semaphores_size);
	this->images_in_flight.resize(this->swapchain_images.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < semaphores_size; i++) {
		if (vkCreateSemaphore(this->device, &semaphoreInfo, nullptr, &this->image_available_semaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(this->device, &semaphoreInfo, nullptr, &this->render_finished_semaphores[i]) != VK_SUCCESS ||
			vkCreateFence(this->device, &fenceInfo, nullptr, &this->in_flight_fences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}

}
void VulkanBaseApp::deleteSwapchain() {
	vkDestroyImageView(this->device, this->depth_image_view, nullptr);
	vkDestroyImage(this->device, this->depth_image, nullptr);
	vkFreeMemory(this->device, this->depth_image_memory, nullptr);

	vkDestroyImageView(this->device, this->color_image_view, nullptr);
	vkDestroyImage(this->device, this->color_image, nullptr);
	vkFreeMemory(this->device, this->color_image_memory, nullptr);

	for (auto framebuffer : this->swapchain_frame_buffers) {
		vkDestroyFramebuffer(this->device, framebuffer, nullptr);
	}

	vkFreeCommandBuffers(this->device, this->command_pool, static_cast<uint32_t>(command_buffers.size()), commandBuffers.data());

	// vkDestroyPipeline(this->device, this->graphics_pipeline, nullptr);
	// vkDestroyPipelineLayout(this->device, this->pipeline_layout, nullptr);
	vkDestroyRenderPass(this->device, this->render_pass, nullptr);

	for (auto imageView : this->swapchain_image_views) {
		vkDestroyImageView(this->device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);

	for (size_t i = 0; i < this->swapchain_images.size(); i++) {
		vkDestroyBuffer(this->device, this->uniform_buffers[i], nullptr);
		vkFreeMemory(this->device, this->uniform_buffers_memory[i], nullptr);
	}

	vkDestroyDescriptorPool(this->device, this->descriptor_pool, nullptr);
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
