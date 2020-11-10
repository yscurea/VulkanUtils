#pragma once

#include "Platform.h"

class Surface
{
private:
	VkSurfaceKHR surface;

public:
	VkSurfaceKHR* getSurface();
	// プラットフォームによって生成する過程が違う、条件コンパイルを使うことになるだろう
	void createSurface(VkInstance* instance, GLFWwindow* window);
};

