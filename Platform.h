#pragma once

//Windows
#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR;
#define GLFW_INCLUDE_VULKAN;
#include <Windows.h>
#endif

#include <vulkan/vulkan.h>