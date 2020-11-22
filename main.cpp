#include "VulkanBaseApp.h"
#include "Object.h"
#include "RenderingObject.h"
#include "Model.h"
#include "UniformBuffer.h"
#include "Camera.h"

// �I�u�W�F�N�g�w���݌v�ŕ`����ǂ������΂����̂����܂����킩��Ȃ�

static std::string model_path = "model/sphere.obj";

class VulkanApp : public VulkanBaseApp {
	Camera* camera = new Camera(this->swapchain_extent.width, this->swapchain_extent.height);
	std::vector<RenderingObject> spheres;
	// �������ߖ�̂��߈�̃��f����S�Ăŋ��L����D�r���I�A�N�Z�X�͕ۏ؂����
	Model unique_model;

	// �p�C�v���C���A����͒P��
	VkPipeline graphics_pipeline;
	VkPipelineLayout pipeline_layout;

	// �e���̗p�̃f�X�N���v�^�Z�b�g
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
		// -------------- ���C�A�E�g�쐬 --------------
		std::array<VkDescriptorSetLayoutBinding, 2> set_layout_bindings{};

		// �萔�o�b�t�@�Ɏg��
		set_layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		set_layout_bindings[0].binding = 0;
		set_layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		set_layout_bindings[0].descriptorCount = 1;

		// �e�N�X�`���T���v���[�Ɏg��
		set_layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		set_layout_bindings[1].binding = 1;
		set_layout_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		set_layout_bindings[1].descriptorCount = 1;

		VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
		descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayoutCI.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
		descriptorLayoutCI.pBindings = set_layout_bindings.data();

		vulkan::utils::checkResult(vkCreateDescriptorSetLayout(this->device, &descriptorLayoutCI, nullptr, &this->descriptor_set_layout));




		// --------------------- pool�쐬 ---------------
		std::array<VkDescriptorPoolSize, 2> descriptor_pool_sizes{};

		// ���̂̐������쐬�����
		descriptor_pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_pool_sizes[0].descriptorCount = 1 + static_cast<uint32_t>(this->spheres.size());

		// ���̂̐������쐬�����\��������i����͑����Ȃ��j
		descriptor_pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_pool_sizes[1].descriptorCount = static_cast<uint32_t>(this->spheres.size());

		VkDescriptorPoolCreateInfo descriptor_pool_create_info = {};
		descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool_create_info.poolSizeCount = static_cast<uint32_t>(descriptor_pool_sizes.size());
		descriptor_pool_create_info.pPoolSizes = descriptor_pool_sizes.data();
		descriptor_pool_create_info.maxSets = static_cast<uint32_t>(this->spheres.size());

		vulkan::utils::checkResult(vkCreateDescriptorPool(this->device, &descriptor_pool_create_info, nullptr, &this->descriptor_pool));



		// -------------------- �f�X�N���v�^�Z�b�g�쐬
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

		// ���_�V�F�[�_
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		// �t���O�����g�V�F�[�_
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		// �p�C�v���C���̃V�F�[�_�S����ݒ�
		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// ���_�V�F�[�_�ɓn�����ݒ肷��
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

		// vulkan�ł͏㉺���]����̂ł����Œ�������
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

		// �r���[�|�[�g�̏��ݒ�
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		// ���X�^���C�Y�̏��ݒ�
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		// �}���`�T���v�����ݒ�
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = this->sample_count_flag_bits;

		// �f�v�X�X�e���V���̏��ݒ�
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		// �p�C�v���C���J���[�u�����h�A�^�b�`�����g�̏��ݒ�
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		// �p�C�v���C���J���[�u�����h�X�e�[�g�̏��ݒ�
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

		// �p�C�v���C�����C�A�E�g�̏��ݒ�
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &this->descriptor_set_layout;

		// �p�C�v���C�����C�A�E�g����
		if (vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &this->pipeline_layout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		// �O���t�B�b�N�X�p�C�v���C���̏���ݒ�
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

		// �O���t�B�b�N�X�p�C�v���C������
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

			//todo: ���_�o�b�t�@�ƃC���f�b�N�X�o�b�t�@�̃o�C���h
			// �����o�b�t�@���g�p����̂ň�񂾂��o�C���h����
			this->unique_model.bind();

			for (auto sphere : this->spheres) {
				// �萔�o�b�t�@���͂��ꂼ��o�C���h����
				vkCmdBindDescriptorSets(this->command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline_layout, 0, 1, &sphere.descriptor_set, 0, nullptr);
				sphere.draw();
			}

			vkCmdEndRenderPass(this->command_buffers[i]);

			vulkan::utils::checkResult(vkEndCommandBuffer(this->command_buffers[i]));
		}
	}
	void prepareUniformBuffers() {
		// �S���̋��̂̒萔�o�b�t�@���쐬���Ċ��蓖�Ă�
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
		// �e���̂̒萔�o�b�t�@���X�V����
		for (auto sphere : this->spheres) {
			// sphere.updateUniformBuffer();
			sphere.matrices.projection = camera->perspective;
			sphere.matrices.view = camera->view;
			position.x += 1.0f;
			sphere.matrices.model = glm::translate(glm::mat4(1.0f), position);
		}
	}
	void updateFrame() override {
		// �萔�o�b�t�@�X�V
		updateUniformBuffers();
	}
	// ���O������S�Ă����ł���
	void prepare() {
		VulkanBaseApp::prepare();
		// ���f���ǂݍ���
		this->loadModel();
		// �萔�o�b�t�@����
		this->prepareUniformBuffers();
		// �f�X�N���v�^�Z�b�g����
		this->prepareDescriptorSets();
		// �O���t�B�b�N�X�p�C�v���C������
		this->createGraphicsPipeline();
		// �R�}���h�o�b�t�@����
		this->prepareCommandBuffers();
	}
	void cleanup() {
		vkDestroyPipeline(this->device, this->graphics_pipeline, nullptr);

		VulkanBaseApp::cleanup();
	}

public:
	VulkanApp(uint32_t sphere_count = 100) {
		this->spheres.resize(sphere_count);
		// vulkan ������
		this->initVulkan();
		// ���X����
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