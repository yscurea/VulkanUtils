#pragma once

#include "Object.h"
#include "Model.h"
#include "MVP.h"
#include "Buffer.h"
#include "Component.h"
#include "Texture.h"

#include <list>
#include <vector>

// �R���X�g���N�^�Œ萔�o�b�t�@���蓖�Ă���
class RenderingObject : public Object
{
public:
	RenderingObject();
	// �V�K�ǂݍ���
	void loadModel(std::string file_path);
	// ���łɓǂݍ���ł��郂�f���̋��L
	void loadModel(Model model);
	// �`��R�}���h
	void draw();

	void start() override;
	void update() {
		for (auto component : this->component_list) {
			component->update();
		}
	}

	// ���_�o�b�t�@�A�C���f�b�N�X�o�b�t�@��
	Model model;
	// �e�s��
	MVP matrices;
	// �萔�o�b�t�@
	Buffer uniform_buffer;
	// �e�N�X�`��
	Texture texture;
	// �f�X�N���v�^�Z�b�g
	VkDescriptorSet descriptor_set;
	// pipeline�Q��
	VkPipeline* graphics_pipeline;
	// �e�V�F�[�_�[�̎Q��
	std::vector<VkShaderModule*> shader_modules;
	// �R���|�l���g
	std::list<Component*> component_list;
};
