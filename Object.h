#pragma once

#include "Platform.h"
#include "utils.h"
#include "Model.h"

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
public:
	// �V�K�ǂݍ���
	void loadModel(std::string file_path);
	// ���łɓǂݍ���ł��郂�f���̋��L
	void loadModel(Model model);
	// �`��R�}���h
	void render();
protected:
	Model model;
	MVP matrices;
	VkPipeline* graphics_pipeline;
	std::vector<VkShaderModule*> shader_modules;
	std::list<Component*> component_list;
};
