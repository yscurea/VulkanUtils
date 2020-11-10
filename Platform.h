#pragma once

//Windows
#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR;
#define GLFW_INCLUDE_VULKAN;
#include <Windows.h>
#endif

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
