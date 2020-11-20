#pragma once
#include <vulkan/vulkan.h>

#include <string.h>
#include <stdexcept>

class Buffer
{
private:
	// �o�b�t�@���m�ۂ���f�o�C�X�̎Q��
	VkDevice* device;
	// ������
	VkDeviceMemory* memory = VK_NULL_HANDLE;
	// �o�b�t�@�{��
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDescriptorBufferInfo descriptor_buffer_info;
	// �傫��
	VkDeviceSize size = 0;
	VkDeviceSize alignment = 0;
	// �}�b�v����Ă���A�h���X
	void* mapped = nullptr;
	// ���Ɏg���o�b�t�@�Ȃ̂�
	VkBufferUsageFlags usage_flags;
	// �������v���p�e�B
	VkMemoryPropertyFlags memory_property_flags;

public:
	// �}�b�v����
	VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	// �}�b�v����������
	void unmap();
	// �o�b�t�@���o�C���h����
	VkResult bind(VkDeviceSize offset = 0);
	// �f�X�N���v�^��ݒ�
	void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	// �R�s�[
	void copyTo(void* data, VkDeviceSize size);
	// ������
	VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	// �j��
	void deleted();
};
