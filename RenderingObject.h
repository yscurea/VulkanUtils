#pragma once

#include "Object.h"
#include "Model.h"
#include "MVP.h"
#include "Buffer.h"
#include "Component.h"
#include "Texture.h"

#include <list>
#include <vector>

// コンストラクタで定数バッファ割り当てたい
class RenderingObject : public Object
{
public:
	RenderingObject();
	// 新規読み込み
	void loadModel(std::string file_path);
	// すでに読み込んであるモデルの共有
	void loadModel(Model model);
	// 描画コマンド
	void draw();

	void start() override;
	void update() {
		for (auto component : this->component_list) {
			component->update();
		}
	}

	// 頂点バッファ、インデックスバッファ等
	Model model;
	// 各行列
	MVP matrices;
	// 定数バッファ
	Buffer uniform_buffer;
	// テクスチャ
	Texture texture;
	// デスクリプタセット
	VkDescriptorSet descriptor_set;
	// pipeline参照
	VkPipeline* graphics_pipeline;
	// 各シェーダーの参照
	std::vector<VkShaderModule*> shader_modules;
	// コンポネント
	std::list<Component*> component_list;
};
