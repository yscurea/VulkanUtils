#include "VulkanBaseApp.h"
#include "Object.h"

class VulkanApp : public VulkanBaseApp {
	std::vector<Object> spheres;
	std::vector<VkPipeline> pipelines;
	void createPipeline();
public:
	VulkanApp() {}
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
	void render() override {
		// update each object uniform buffers
		updateUniformBuffers();

		VulkanBaseApp::prepareFrame();
		this->submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffers[current_buffer];
		if (vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS) {
			throw std::runtime_error("failed to queue submit");
		}
		VulkanBaseApp::submitFrame();
	}

	void makeCommand() override {
		// ‹…‘Ì‘S•”•`‰æ‚·‚é
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