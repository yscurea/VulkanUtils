#include "VulkanBaseApp.h"
#include "Object.h"
#include "RenderingObject.h"
#include "Model.h"
#include "UniformBuffer.h"
#include "Camera.h"

// オブジェクト指向設計で描画をどう扱えばいいのかいまいちわからない

static std::string model_path = "model/sphere.obj";

class VulkanApp : public VulkanBaseApp {
	Camera* camera = new Camera(this->swapchain_extent.width, this->swapchain_extent.height);
	std::vector<RenderingObject> spheres;
	// メモリ節約のため一つのモデルを全てで共有する．排他的アクセスは保証される
	Model unique_model;

	// パイプライン、今回は単一
	VkPipeline graphics_pipeline;
	VkPipelineLayout pipeline_layout;

	// 各球体用のデスクリプタセット
	VkDescriptorPool descriptor_pool;
	VkDescriptorSetLayout descriptor_set_layout;
	VkDescriptorSet descriptor_set;

	VkClearColorValue default_clear_color = { { 0.025f, 0.025f, 0.025f, 1.0f } };

	void loadModel() {
		unique_model.load(model_path);
		for (auto sphere : this->spheres) {
			sphere.loadModel(this->unique_model);
		}
	}
	void prepareDescriptorSets() {
		// -------------- レイアウト作成 --------------
		std::array<VkDescriptorSetLayoutBinding, 2> set_layout_bindings{};

		// 定数バッファに使う
		set_layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		set_layout_bindings[0].binding = 0;
		set_layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		set_layout_bindings[0].descriptorCount = 1;

		// テクスチャサンプラーに使う
		set_layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		set_layout_bindings[1].binding = 1;
		set_layout_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		set_layout_bindings[1].descriptorCount = 1;

		VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
		descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayoutCI.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
		descriptorLayoutCI.pBindings = set_layout_bindings.data();

		vulkan::utils::checkResult(vkCreateDescriptorSetLayout(this->device, &descriptorLayoutCI, nullptr, &this->descriptor_set_layout));




		// --------------------- pool作成 ---------------
		std::array<VkDescriptorPoolSize, 2> descriptor_pool_sizes{};

		// 球体の数だけ作成される
		descriptor_pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_pool_sizes[0].descriptorCount = 1 + static_cast<uint32_t>(this->spheres.size());

		// 球体の数だけ作成される可能性がある（今回は多分ない）
		descriptor_pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_pool_sizes[1].descriptorCount = static_cast<uint32_t>(this->spheres.size());

		VkDescriptorPoolCreateInfo descriptor_pool_create_info = {};
		descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool_create_info.poolSizeCount = static_cast<uint32_t>(descriptor_pool_sizes.size());
		descriptor_pool_create_info.pPoolSizes = descriptor_pool_sizes.data();
		descriptor_pool_create_info.maxSets = static_cast<uint32_t>(this->spheres.size());

		vulkan::utils::checkResult(vkCreateDescriptorPool(this->device, &descriptor_pool_create_info, nullptr, &this->descriptor_pool));



		// -------------------- デスクリプタセット作成
		for (auto& sphere : this->spheres) {
			VkDescriptorSetAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocateInfo.descriptorPool = this->descriptor_pool;
			allocateInfo.descriptorSetCount = 1;
			allocateInfo.pSetLayouts = &this->descriptor_set_layout;
			vulkan::utils::checkResult(vkAllocateDescriptorSets(device, &allocateInfo, &sphere.descriptor_set));

			std::array<VkWriteDescriptorSet, 2> write_descriptor_sets{};

			// Binding 0: RenderingObject uniform buffer
			write_descriptor_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_descriptor_sets[0].dstSet = sphere.descriptor_set;
			write_descriptor_sets[0].dstBinding = 0;
			write_descriptor_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			write_descriptor_sets[0].pBufferInfo = &sphere.uniform_buffer.descriptor_buffer_info;
			write_descriptor_sets[0].descriptorCount = 1;

			// Binding 1: RenderingObject texture
			write_descriptor_sets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_descriptor_sets[1].dstSet = sphere.descriptor_set;
			write_descriptor_sets[1].dstBinding = 1;
			write_descriptor_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_descriptor_sets[1].pImageInfo = &sphere.texture.descriptor_image_info;
			write_descriptor_sets[1].descriptorCount = 1;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
		}

	}
	void createGraphicsPipeline() {
		auto vertShaderCode = vulkan::utils::readFile("shaders/vert.spv");
		auto fragShaderCode = vulkan::utils::readFile("shaders/frag.spv");

		VkShaderModule vertShaderModule = vulkan::utils::createShaderModule(this->device, vertShaderCode);
		VkShaderModule fragShaderModule = vulkan::utils::createShaderModule(this->device, fragShaderCode);

		// 頂点シェーダ
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		// フラグメントシェーダ
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		// パイプラインのシェーダ全部を設定
		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// 頂点シェーダに渡すやつを設定する
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// vulkanでは上下反転するのでここで調整する
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = (float)this->swapchain_extent.height;
		viewport.width = (float)this->swapchain_extent.width;
		viewport.height = -(float)this->swapchain_extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = this->swapchain_extent;

		// ビューポートの情報設定
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		// ラスタライズの情報設定
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		// マルチサンプル情報設定
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = this->sample_count_flag_bits;

		// デプスステンシルの情報設定
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		// パイプラインカラーブレンドアタッチメントの情報設定
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		// パイプラインカラーブレンドステートの情報設定
		VkPipelineColorBlendStateCreateInfo color_blending{};
		color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blending.logicOpEnable = VK_FALSE;
		color_blending.logicOp = VK_LOGIC_OP_COPY;
		color_blending.attachmentCount = 1;
		color_blending.pAttachments = &colorBlendAttachment;
		color_blending.blendConstants[0] = 0.0f;
		color_blending.blendConstants[1] = 0.0f;
		color_blending.blendConstants[2] = 0.0f;
		color_blending.blendConstants[3] = 0.0f;

		// パイプラインレイアウトの情報設定
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &this->descriptor_set_layout;

		// パイプラインレイアウト生成
		if (vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &this->pipeline_layout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		// グラフィックスパイプラインの情報を設定
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &color_blending;
		pipelineInfo.layout = this->pipeline_layout;
		pipelineInfo.renderPass = this->render_pass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		// グラフィックスパイプライン生成
		if (vkCreateGraphicsPipelines(this->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->graphics_pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}
	}
	void prepareCommandBuffers() {
		VkCommandBufferBeginInfo command_buffer_begin_info{};
		command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		VkClearValue clearValues[2];
		clearValues[0].color = this->default_clear_color;
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo render_pass_begin_info{};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = this->render_pass;
		render_pass_begin_info.renderArea.offset.x = 0;
		render_pass_begin_info.renderArea.offset.y = 0;
		render_pass_begin_info.renderArea.extent.width = this->window_width;
		render_pass_begin_info.renderArea.extent.height = this->window_height;
		render_pass_begin_info.clearValueCount = 2;
		render_pass_begin_info.pClearValues = clearValues;

		for (int32_t i = 0; i < this->command_buffers.size(); ++i) {
			render_pass_begin_info.framebuffer = this->swapchain_frame_buffers[i];

			vulkan::utils::checkResult(vkBeginCommandBuffer(this->command_buffers[i], &command_buffer_begin_info));

			vkCmdBeginRenderPass(this->command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(this->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphics_pipeline);

			VkViewport viewport;
			viewport.height = (float)this->window_height;
			viewport.width = (float)this->window_width;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(this->command_buffers[i], 0, 1, &viewport);

			VkRect2D scissor;
			scissor.extent = this->swapchain_extent;
			scissor.offset = { 0,0 };
			vkCmdSetScissor(this->command_buffers[i], 0, 1, &scissor);

			VkDeviceSize offsets[1] = { 0 };

			//todo: 頂点バッファとインデックスバッファのバインド
			// 同じバッファを使用するので一回だけバインドする
			this->unique_model.bind();

			for (auto sphere : this->spheres) {
				// 定数バッファ等はそれぞれバインドする
				vkCmdBindDescriptorSets(this->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline_layout, 0, 1, &sphere.descriptor_set, 0, nullptr);
				sphere.draw();
			}

			vkCmdEndRenderPass(this->command_buffers[i]);

			vulkan::utils::checkResult(vkEndCommandBuffer(this->command_buffers[i]));
		}
	}
	void prepareUniformBuffers() {
		// 全部の球体の定数バッファを作成して割り当てる
		for (auto sphere : this->spheres) {
			vulkan::utils::createBuffer(
				this->device,
				this->physical_device,
				sizeof(MVP),
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				sphere.uniform_buffer.buffer,
				sphere.uniform_buffer.memory
			);
		}
	}
	void updateUniformBuffers() {
		glm::vec3 position = { 0.0f,0.0f,0.0f };
		// 各球体の定数バッファを更新する
		for (auto sphere : this->spheres) {
			// sphere.updateUniformBuffer();
			sphere.matrices.projection = camera->perspective;
			sphere.matrices.view = camera->view;
			position.x += 1.0f;
			sphere.matrices.model = glm::translate(glm::mat4(1.0f), position);
		}
	}
	void updateFrame() override {
		// 定数バッファ更新
		updateUniformBuffers();
	}
	// 事前準備を全てここでする
	void prepare() {
		VulkanBaseApp::prepare();
		// モデル読み込み
		this->loadModel();
		// 定数バッファ準備
		this->prepareUniformBuffers();
		// デスクリプタセット準備
		this->prepareDescriptorSets();
		// グラフィックスパイプライン生成
		this->createGraphicsPipeline();
		// コマンドバッファ準備
		this->prepareCommandBuffers();
	}
	void cleanup() {
		vkDestroyPipeline(this->device, this->graphics_pipeline, nullptr);

		VulkanBaseApp::cleanup();
	}

public:
	VulkanApp(uint32_t sphere_count = 100) {
		this->spheres.resize(sphere_count);
		// vulkan 初期化
		this->initVulkan();
		// 諸々準備
		this->prepare();
	}
	~VulkanApp() {
		this->cleanup();
	}
};

int main() {
	uint32_t sphere_count = 100;
	VulkanApp* app = new VulkanApp(sphere_count);

	// main loop
	app->renderLoop();

	// delete app;
}