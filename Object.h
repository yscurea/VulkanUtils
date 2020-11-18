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
	// 新規読み込み
	void loadModel(std::string file_path);
	// すでに読み込んであるモデルの共有
	void loadModel(Model model);
	// 描画コマンド
	void render();
protected:
	Model model;
	MVP matrices;
	VkPipeline* graphics_pipeline;
	std::vector<VkShaderModule*> shader_modules;
	std::list<Component*> component_list;
};
