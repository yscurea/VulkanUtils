#include "Buffer.h"


VkResult Buffer::map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
	return vkMapMemory(this->device, this->memory, offset, size, 0, &this->mapped);
}


void Buffer::unmap() {
	if (mapped) {
		vkUnmapMemory(this->device, this->memory);
		mapped = nullptr;
	}
}

VkResult Buffer::bind(VkDeviceSize offset) {
	return vkBindBufferMemory(this->device, this->buffer, this->memory, offset);
}


void Buffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset) {
	this->descriptor_buffer_info.offset = offset;
	this->descriptor_buffer_info.buffer = this->buffer;
	this->descriptor_buffer_info.range = size;
}



void Buffer::copyTo(void* data, VkDeviceSize size) {
	if (this->mapped == nullptr) {
		throw std::runtime_error("failed to copy buffer");
	}
	memcpy(this->mapped, data, size);
}
