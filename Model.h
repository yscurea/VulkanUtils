#pragma once

#include <vulkan/vulkan.h>

#include <string>

#include "Vertex.h"

class Model
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
public:
	void load(std::string file_path);
};

