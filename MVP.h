#pragma once

#include <glm/glm.hpp>

// ������
struct MVP {
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 model;
};
