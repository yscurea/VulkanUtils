#pragma once

#include <glm/glm.hpp>

// ���ʂ̃r���[�s��ƃv���W�F�N�V�����s����g�p����ق��������������͂����H

struct UniformBuffer {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 projection;
};
