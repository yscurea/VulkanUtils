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
protected:

public:
	Object() {}
	~Object() {}

	virtual void start() {}
	virtual void update() {}
};

class RenderingObject : public Object
{
protected:
	MVP matrices;
	VkPipeline* graphics_pipeline;
	std::vector<VkShaderModule*> shader_modules;
	std::list<Component*> component_list;
};
