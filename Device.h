#pragma once

#include "Platform.h"
#include "utils.h"

#include <vector>
#include <string>
#include <optional>

class Device
{
private:
	// インスタンスの参照
	VkInstance* instance;
	// gpu
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	// 論理デバイス
	VkDevice logical_device;
	// プロパティー
	VkPhysicalDeviceProperties	properties;
	// 特徴
	VkPhysicalDeviceFeatures features;
	// 取得できる特徴
	VkPhysicalDeviceFeatures enabled_features;
	// メモリプロパティー
	VkPhysicalDeviceMemoryProperties memory_properties;
	std::optional<uint32_t> graphics_queue_index;
	std::optional<uint32_t> compute_queue_index;
	// std::optional<uint32_t> transfer_queue_index;
	VkQueue graphics_queue;
	VkQueue present_queue;
	std::vector<VkQueueFamilyProperties> queue_family_properties;
	std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	std::vector<const char*> supported_extensions;
	VkSampleCountFlagBits sample_count_flag_bits;

	// command pool
	VkCommandPool command_pool = VK_NULL_HANDLE;
	// prepare buffers for the number of images in the swapchain
	std::vector<VkCommandBuffer> command_buffers;
public:
	Device(VkInstance* instance);
	~Device();

	void			selectPhysicalDevice(bool (*isSuitable)(VkPhysicalDevice gpu));
	VkResult        createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions, void* pNextChain, bool useSwapChain = true, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
	void			findDeviceQueues(VkSurfaceKHR* surface) const;

	VkCommandPool   createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	VkResult		createCommandBuffer();

	VkDevice* getLogicalDevice();

	uint32_t        getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr) const;
	uint32_t        getQueueFamilyIndex(VkQueueFlagBits queueFlags) const;
	VkResult        createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr);
	VkResult        createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, vks::Buffer* buffer, VkDeviceSize size, void* data = nullptr);
	// void            copyBuffer(vks::Buffer* src, vks::Buffer* dst, VkQueue queue, VkBufferCopy* copyRegion = nullptr);
	bool            extensionSupported(std::vector<const char*> extension);
	// VkFormat        getSupportedDepthFormat(bool checkSamplingSupport);

private:
	bool			isDeviceSuitable(VkPhysicalDevice device);
	VkSampleCountFlagBits getMaxUsableSampleCount();
};
