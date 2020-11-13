#include "Buffer.h"


VkResult Buffer::map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
	return vkMapMemory(*device, *memory, offset, size, 0, &mapped);
}


void Buffer::unmap() {
	if (mapped) {
		vkUnmapMemory(*device, *memory);
		mapped = nullptr;
	}
}

VkResult Buffer::bind(VkDeviceSize offset) {
	return vkBindBufferMemory(*device, buffer, *memory, offset);
}


void Buffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset) {
	descriptor.offset = offset;
	descriptor.buffer = buffer;
	descriptor.range = size;
}



void Buffer::copyTo(void* data, VkDeviceSize size) {
	if (mapped == nullptr) {
		throw std::runtime_error("failed to copy buffer");
	}
	memcpy(mapped, data, size);
}
