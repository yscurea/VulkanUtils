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



void VulkanBaseApp::createSynchronization() {
	VkFenceCreateInfo fence_create_info{};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	wait_fences.resize(command_buffers.size());
	for (auto& fence : wait_fences) {

		if ((vkCreateFence(*this->vulkan_device->getLogicalDevice(), &fence_create_info, nullptr, &fence)) != VK_SUCCESS) {
			throw std::runtime_error("failed to create fence");
		}
	}
}

void VulkanBaseApp::setupRenderPass() {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = this->swapchain_image_format;
	colorAttachment.samples = this->msaaSamples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = this->findDepthFormat();
	depthAttachment.samples = this->msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = swapChainImageFormat;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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

	if (vkCreateRenderPass(*this->device, &renderPassInfo, nullptr, &this->render_pass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}


void VulkanBaseApp::createCommandPool() {
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = vulkan_device->getGraphicsQueueIndex();
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (vkCreateCommandPool(*this->vulkan_device->getLogicalDevice(), &cmdPoolInfo, nullptr, &command_pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool");
	}
}

void VulkanBaseApp::renderFrame() {

}

void VulkanBaseApp::renderLoop() {
	while (!glfwWindowShouldClose(window)) {
		// ÉCÉxÉìÉgèàóùÇÇµÇƒ
		glfwPollEvents();
		// ï`âÊÇ∑ÇÈ
		renderFrame();
	}
}


void presentFrame() {
	VkResult result = swapchain.queuePresent(queue, currentBuffer, semaphores.renderComplete);
	if (!((result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR))) {
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			windowResize();
			return;
		}
		else {
			VK_CHECK_RESULT(result);
		}
	}
	VK_CHECK_RESULT(vkQueueWaitIdle(queue));
}
