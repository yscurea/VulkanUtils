#pragma once

#include <glm/glm.hpp>

// 共通のビュー行列とプロジェクション行列を使用するほうがメモリ効率はいい？

struct UniformBuffer {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 projection;
};
