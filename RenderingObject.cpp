#include "RenderingObject.h"

RenderingObject::RenderingObject() {

}
void RenderingObject::draw(VkCommandBuffer command_buffer, bool is_bound_buffers) {
	if (is_bound_buffers) {
		this->model.draw(command_buffer);
	}
}
void RenderingObject::loadModel(std::string file_path) {
	this->model.load(file_path);
}

void RenderingObject::loadModel(Model model) {
	this->model = model;
}
