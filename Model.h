#pragma once

#include <vulkan/vulkan.h>

#include <string>

#include "Vertex.h"

class Model
{
	// ���_
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	// �e�N�X�`��
	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
public:
	void load(std::string file_path);
};

