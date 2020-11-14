#include "VulkanBaseApp.h"
#include "Object.h"

class VulkanApp : public VulkanBaseApp {
	std::vector<Object> spheres;
public:

	void prepare() {
		VulkanBaseApp::prepare();
		this->spheres.resize(100);
	}

	void prepareUniformBuffers() {
		for (auto& object : this->spheres) {
			// create uniform buffer
		}
		this->updateUniformBuffers();
	}
	void draw() {
		// submit command
	}
	void preparePipeline() {

	}
	void prepareCommand() {

	}
	void prepareUniformBuffers() {

	}
	void updateUniformBuffers() {

	}
	virtual void render() {
		// update each object uniform buffers
		updateUniformBuffers();

		VulkanExampleBase::prepareFrame();
		this->submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &drawCmdBuffers[currentBuffer];
		if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
			throw std::runtime_error("failed to queue submit");
		}
		VulkanExampleBase::submitFrame();
	}
};

int main() {
	VulkanApp* app = new VulkanApp();

	// initialize vulkan setting
	app->initVulkan();

	// prepare before start rendering
	app->prepare();

	// main loop
	app->renderLoop();

	// delete app;
}