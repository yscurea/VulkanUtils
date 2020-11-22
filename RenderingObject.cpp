#include "RenderingObject.h"

RenderingObject::RenderingObject() {

}
void RenderingObject::draw(VkCommandBuffer command_buffer, bool is_bound_buffers) {
	this->model.draw(command_buffer, is_bound_buffers);
}
void RenderingObject::loadModel(std::string file_path) {
	this->model.load(file_path);
}

void RenderingObject::loadModel(Model model) {
	this->model = model;
}


void RenderingObject::update() {
	// �{���͂����ŕ`�施�߂�^������
	/*
		for (auto component : this->component_list) {
			component->update();
		}
	*/
}
