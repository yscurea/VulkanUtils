#pragma once
#include <vulkan/vulkan.h>

#include <string.h>
#include <stdexcept>

class Buffer
{
	// バッファを確保するデバイスの参照
	VkDevice* device;
	VkDeviceMemory* memory = VK_NULL_HANDLE;

	// バッファ
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDescriptorBufferInfo descriptor_buffer_info;
	// 大きさ
	VkDeviceSize size = 0;
	VkDeviceSize alignment = 0;
	// マップされているアドレス
	void* mapped = nullptr;
	// 何に使うバッファなのか
	VkBufferUsageFlags usage_flags;
	VkMemoryPropertyFlags memory_property_flags;

	// マップする
	VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	// マップを解除する
	void unmap();
	// バッファをバインドする
	VkResult bind(VkDeviceSize offset = 0);
	// デスクリプタを設定
	void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	// コピー
	void copyTo(void* data, VkDeviceSize size);
	// 無効化
	VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	// 破棄
	void destroy();
};

