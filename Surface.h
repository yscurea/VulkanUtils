#pragma once

#include "Platform.h"

class Surface
{
private:
	VkSurfaceKHR surface;

public:
	VkSurfaceKHR* getSurface();
	// �v���b�g�t�H�[���ɂ���Đ�������ߒ����Ⴄ�A�����R���p�C�����g�����ƂɂȂ邾�낤
	void createSurface(VkInstance* instance, GLFWwindow* window);
};

