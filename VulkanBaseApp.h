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


class VulkanBaseApp {
public:
	// constructor
	VulkanBaseApp();
	virtual ~VulkanBaseApp();

	virtual void makeCommand(VkCommandBuffer command_buffer) {}

	// 必要最低限の初期化
	void initVulkan();
	// 継承先において追加で必要な処理
	void prepare();


	void renderFrame();
	// 毎フレームの準備
	void prepareFrame();
	// 毎フレーム転送
	void submitFrame(uint32_t image_index);
	// 毎フレーム表示
	void presentFrame(uint32_t image_index, VkSemaphore* render_finished_semaphores);
	virtual void updateFrame() = 0;
	virtual void renderLoop();

	// 必要な解放処理
	void cleanup();
protected:
	// ------------ instance ------------
	VkInstance instance;
	std::vector<const char*> supported_instance_extensions;

	void createInstance();
	void deleteInstance();


#ifdef _DEBUG
	// ------------- debug --------------
	std::vector<const char*> validation_layer_names = { "VK_LAYER_KHRONOS_validation" };
	VkDebugUtilsMessengerEXT debug_messenger;
	void setupDebugMessenger();
#endif


	// ------------ device -------------
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice device;
	VkPhysicalDeviceProperties	properties;
	VkPhysicalDeviceFeatures features;
	VkPhysicalDeviceMemoryProperties memory_properties;
	std::optional<uint32_t> graphics_queue_index;
	std::optional<uint32_t> present_queue_index;
	std::optional<uint32_t> compute_queue_index;
	VkQueue graphics_queue;
	VkQueue present_queue;
	std::vector<VkQueueFamilyProperties> queue_family_properties;
	std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	std::vector<const char*> supported_extensions;
	VkSampleCountFlagBits sample_count_flag_bits;
	VkSubmitInfo submit_info;

	void selectPhysicalDevice();
	void createLogicalDevice();


	// ------------- surface ---------------
	// todo : プラットフォームごとによる条件コンパイル
	VkSurfaceKHR surface;
	VkSurfaceFormatKHR surface_format;
	void createSurface();
	void recreateSurface();
	void deleteSurface();

	// window
	uint32_t window_width = 800;
	uint32_t window_height = 600;
	GLFWwindow* window;
	void createWindow();
	void windowResize(); // call back

	// ------------- swapchain ---------------
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	vulkan::SwapChainSupportDetails swapchain_support_details;
	VkFormat swapchain_image_format;
	uint32_t swapchain_image_count;
	std::vector<VkImage> swapchain_images;
	std::vector<VkImageView> swapchain_image_views;
	VkExtent2D swapchain_extent;
	VkSampleCountFlagBits sample_count_flag_bits = VK_SAMPLE_COUNT_1_BIT;
	// syncronization
	std::vector<VkSemaphore> image_available_semaphores;
	std::vector<VkSemaphore> render_finished_semaphores;
	uint32_t semaphores_size = 2;
	bool framebuffer_resized = false;

	void createSemaphores();

	std::vector<VkFence> in_flight_fences;
	std::vector<VkFence> images_in_flight;
	size_t fences_size = 2;
	size_t current_frame = 0;

	void createFences();

	// frame buffer
	std::vector<VkFramebuffer> swapchain_frame_buffers;
	// color buffer
	VkImage color_image;
	VkDeviceMemory color_image_memory;
	VkImageView color_image_view;
	void createColorResources();
	// depth buffer
	VkImage depth_image;
	VkDeviceMemory depth_image_memory;
	VkImageView depth_image_view;
	VkFormat depth_format;
	void createDepthResources();
	// stencil buffer
	VkImage stencil_image;
	VkDeviceMemory stencil_image_memory;
	VkImageView stencil_image_view;

	void createSwapchain();
	void recreateSwapchain();
	void deleteSwapchain();

	// ------------- renderpass ------------
	VkRenderPass render_pass;
	// 必要に応じてrennderpassの生成法を変えるためvirtual MSAA etc
	virtual void setupRenderPass();

	// -------------- command --------------
	// pool
	VkCommandPool command_pool;
	// buffers swapchainの枚数だけ用意する
	std::vector<VkCommandBuffer> command_buffers;

	void createCommandPool();
	virtual void createCommandBuffers();


	// -------------- resources -----------
	std::vector<VkShaderModule> shader_modules;
	VkPipelineCache pipelineCache;
	VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
};
