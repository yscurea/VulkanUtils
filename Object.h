#pragma once

#include "Platform.h"
#include "utils.h"

#include <list>

struct MVP {
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 model;
};

class Component {

};


class Object
{
private:
	// ˆÊ’u
	MVP matrices;
	VkPipeline graphics_pipeline;
	VkShaderModule* shader;

	std::list<Component*> component_list;
public:
	Object();
	~Object();

	void Update();
};

