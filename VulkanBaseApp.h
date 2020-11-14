#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <array>
#include <numeric>
#include <ctime>
#include <iostream>
#include <chrono>
#include <random>
#include <algorithm>
#include <sys/stat.h>
#include <string>
#include <numeric>
#include <array>

#include "Instance.h"
#include "Device.h"
#include "Surface.h"
#include "Swapchain.h"
#include "utils.h"

struct ApplicationCreateInfo {
	const char* application_name;
};


class VulkanBaseApp {
public:
	// constructor
	VulkanBaseApp();
	virtual ~VulkanBaseApp();

	virtual void makeCommand(VkCommandBuffer command_buffer) {}

	// 必要最低限の初期化
	void initVulkan();
	void prepare();
	virtual void render() = 0;
	void renderLoop();
	void cleanup();
protected:

	// インスタンス
	VkInstance instance;
	std::vector<const char*> supported_instance_extensions;
	void createInstance();
	void deleteInstance();

	// デバイス
	VkPhysicalDevice physical_device;
	VkDevice device;
	VkPhysicalDeviceProperties	properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties memory_properties;
	std::optional<uint32_t> graphics_queue_index;
	std::optional<uint32_t> compute_queue_index;
	VkQueue graphics_queue;
	VkQueue present_queue;
	std::vector<VkQueueFamilyProperties> queue_family_properties;
	std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	std::vector<const char*> supported_extensions;
	VkSampleCountFlagBits sample_count_flag_bits;
	void selectPhysicalDevice();
	void createLogicalDevice();

	// surface
	VkSurfaceKHR surface;
	void createSurface();
	void recreateSurface();
	uint32_t width = 800;
	uint32_t height = 600;
	GLFWwindow* window;
	void createWindow();
	void windowResize();

	// swapchain
	VkFormat swapchain_image_format;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	std::vector<VkImage> swapchain_images;
	VkExtent2D swapchain_extent;
	VkSampleCountFlagBits sample_count_flag_bits = VK_SAMPLE_COUNT_1_BIT;
	// syncronization
	std::vector<VkSemaphore> image_available_semaphores;
	std::vector<VkSemaphore> render_finished_semaphores;
	std::vector<VkFence> in_flight_fences;
	std::vector<VkFence> images_in_flight;
	size_t current_frame = 0;
	// frame buffer
	std::vector<VkFramebuffer> swapchain_frame_buffers;
	// color buffer
	VkImage color_image;
	VkDeviceMemory color_image_memory;
	VkImageView color_image_view;
	// depth buffer
	VkImage depth_image;
	VkDeviceMemory depth_image_memory;
	VkImageView depth_image_view;
	// stencil buffer
	VkImage stencil_image;
	VkDeviceMemory stencil_image_memory;
	VkImageView stencil_image_view;
	void createSwapchain();
	void recreateSwapchain();
	void deleteSwapchain();

	// レンダーパス
	VkRenderPass render_pass;
	virtual void setupRenderPass();

	// コマンドプール
	VkCommandPool command_pool;
	void createCommandPool();

	// コマンドバッファ
	std::vector<VkCommandBuffer> command_buffers;
	virtual void createCommandBuffer();


	void createSynchronization();

	VkSubmitInfo submit_info;

	void renderFrame();

	void present();

	std::vector<VkShaderModule> shader_modules;
	VkPipelineCache pipelineCache;


	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;


#ifdef _DEBUG
	std::vector<const char*> validation_layer_names = { "VK_LAYER_KHRONOS_validation" };

	VKAPI_ATTR VkBool32 VKAPI_CALL messageCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t srcObject,
		size_t location,
		int32_t msgCode,
		const char* pLayerPrefix,
		const char* pMsg,
		void* pUserData
	);
	void setupDebugging(
		VkInstance instance,
		VkDebugReportFlagsEXT flags,
		VkDebugReportCallbackEXT callBack
	);
	void deleteDebugCallback(VkInstance instance);
#endif
};
