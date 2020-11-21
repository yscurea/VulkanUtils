#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>

class Camera {
public:
	Camera(int window_width, float window_height) {
		this->window_width = window_width;
		this->window_height = window_height;
		this->near_clip = 0.1f;
		this->far_clip = 100.0f;

		this->perspective = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, this->near_clip, this->far_clip);
	}
	int window_height;
	int window_width;
	glm::mat4 perspective;
	glm::mat4 view;
	float near_clip;
	float far_clip;
	void updateCameraPosition();
};

