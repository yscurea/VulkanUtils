#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <array>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 tex_coord;

	static VkVertexInputBindingDescription getBindingDescription();

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

	bool operator==(const Vertex& other) const;
};

