#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>


class Camera {
private:
	glm::vec3 position;
	glm::vec3 rotation;
	float near_clip;
	float far_clip;
public:
	void updateCameraPosition();
};

