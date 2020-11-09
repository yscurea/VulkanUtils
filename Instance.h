#pragma once

#include "Platform.h"

#include <vector>

class Instance
{
private:
	VkInstance instance;
	std::vector<const char*> instance_extensions;
	bool isDebugMode = false;
	std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" };
public:
	Instance(bool isDebug);

	VkInstance* getInstance();
	void createInstance();
};
