#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <cassert>
#include <vector>
#include <set>
#include <optional>
#include <array>

#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "VulkanInitializer.h"

namespace vk::utils {

	const static std::vector<const char*> device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};


	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};



	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

			return attributeDescriptions;
		}

		bool operator==(const Vertex& other) const {
			return pos == other.pos && color == other.color && texCoord == other.texCoord;
		}
	};


	struct Buffer {
		VkDevice device;
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDescriptorBufferInfo descriptor;
		int32_t count = 0;
		void* mapped = nullptr;
	};

	struct Image {
		VkImage image;
		VkImageView image_view;
	};

	struct Texture {
		uint32_t mipLevels;
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView;
		VkSampler textureSampler;
	};

	void checkResult(VkResult result);
	bool checkValidationLayerSupport(std::vector<const char*> validation_layers) {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validation_layers) {
			bool layerFound = false;
			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}
			if (!layerFound) {
				return false;
			}
		}
		return true;
	}

	namespace instance {

		void createInstance(const char* application_name, VkInstance* instance) {
			std::vector<const char*> extensions;

			// アプリケーション情報を初期化
			VkApplicationInfo appInfo = vk::utils::initializer::getAppInfo();
			appInfo.pApplicationName = application_name;

			// 拡張情報の取得
			std::vector<VkExtensionProperties> props;
			{
				uint32_t count = 0;
				vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
				props.resize(count);
				vkEnumerateInstanceExtensionProperties(nullptr, &count, props.data());

				for (const auto& v : props)
				{
					extensions.push_back(v.extensionName);
				}
			}
			VkInstanceCreateInfo ci{};
			ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			ci.enabledExtensionCount = uint32_t(extensions.size());
			ci.ppEnabledExtensionNames = extensions.data();
			ci.pApplicationInfo = &appInfo;

#ifdef NDEBUG
#else
			// レイヤー、検証レイヤーを有効化
			const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
			ci.enabledLayerCount = 1;
			ci.ppEnabledLayerNames = layers;
#endif
			auto result = vkCreateInstance(&ci, nullptr, instance);
			std::cout << "create instance" << std::endl;
			checkResult(result);
		}

	}

	namespace device {

		VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice gpu) {
			VkPhysicalDeviceProperties physicalDeviceProperties;
			vkGetPhysicalDeviceProperties(gpu, &physicalDeviceProperties);

			VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
			if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
			if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
			if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
			if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
			if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
			if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

			return VK_SAMPLE_COUNT_1_BIT;
		}

		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
			QueueFamilyIndices indices;

			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

			int i = 0;
			for (const auto& queueFamily : queueFamilies) {
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					indices.graphicsFamily = i;
				}
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
				if (presentSupport) {
					indices.presentFamily = i;
				}
				if (indices.isComplete()) {
					break;
				}
				i++;
			}
			return indices;
		}

		bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
			uint32_t extensionCount;
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

			std::set<std::string> requiredExtensions(device_extensions.begin(), device_extensions.end());

			for (const auto& extension : availableExtensions) {
				requiredExtensions.erase(extension.extensionName);
			}

			return requiredExtensions.empty();
		}

		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
			SwapChainSupportDetails details;

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

			if (formatCount != 0) {
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

			if (presentModeCount != 0) {
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
			}

			return details;
		}

		bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
			QueueFamilyIndices indices = findQueueFamilies(device, surface);

			bool extensionsSupported = checkDeviceExtensionSupport(device);

			bool swapChainAdequate = false;
			if (extensionsSupported) {
				SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
				swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
			}

			VkPhysicalDeviceFeatures supportedFeatures;
			vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

			return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
		}

		void selectPhysicalDevice(VkPhysicalDevice* gpu, VkSampleCountFlagBits* msaaSamples, VkInstance instance, VkSurfaceKHR surface) {
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

			if (deviceCount == 0) {
				throw std::runtime_error("failed to find GPUs with Vulkan support!");
			}

			std::vector<VkPhysicalDevice> devices(deviceCount);
			vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

			for (const auto& device : devices) {
				if (isDeviceSuitable(device, surface)) {
					*gpu = device;
					*msaaSamples = getMaxUsableSampleCount(*gpu);
					break;
				}
			}

			if (*gpu == VK_NULL_HANDLE) {
				throw std::runtime_error("failed to find a suitable GPU!");
			}
		}

		uint32_t searchGraphicsQueueIndex(VkPhysicalDevice physical_device)
		{
			uint32_t propCount;
			vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &propCount, nullptr);
			std::vector<VkQueueFamilyProperties> props(propCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &propCount, props.data());

			uint32_t graphicsQueue = ~0u;
			for (uint32_t i = 0; i < propCount; i++) {
				if (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					graphicsQueue = i;
					break;
				}
			}
			return graphicsQueue;
		}


		VkResult createLogicalDevice(VkDevice* device, VkPhysicalDevice physical_device, VkSurfaceKHR surface, VkQueue* graphics_queue, VkQueue* present_queue) {
			QueueFamilyIndices indices = findQueueFamilies(physical_device, surface);

			std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
			std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

			float queuePriority = 1.0f;
			for (uint32_t queueFamily : uniqueQueueFamilies) {
				VkDeviceQueueCreateInfo queueCreateInfo{};
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = queueFamily;
				queueCreateInfo.queueCount = 1;
				queueCreateInfo.pQueuePriorities = &queuePriority;
				queueCreateInfos.push_back(queueCreateInfo);
			}

			VkPhysicalDeviceFeatures deviceFeatures{};
			deviceFeatures.samplerAnisotropy = VK_TRUE;

			VkDeviceCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

			createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
			createInfo.pQueueCreateInfos = queueCreateInfos.data();

			createInfo.pEnabledFeatures = &deviceFeatures;

			createInfo.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
			createInfo.ppEnabledExtensionNames = device_extensions.data();

#ifdef NDEBUG
			createInfo.enabledLayerCount = 0;
#else
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
#endif

			if (vkCreateDevice(physical_device, &createInfo, nullptr, device) != VK_SUCCESS) {
				throw std::runtime_error("failed to create logical device!");
			}

			vkGetDeviceQueue(*device, indices.graphicsFamily.value(), 0, graphics_queue);
			vkGetDeviceQueue(*device, indices.presentFamily.value(), 0, present_queue);
		}
	}

	namespace debug {

	}

	namespace swapchain {
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
			SwapChainSupportDetails details;

			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

			uint32_t formatCount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

			if (formatCount != 0) {
				details.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
			}

			uint32_t presentModeCount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

			if (presentModeCount != 0) {
				details.presentModes.resize(presentModeCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
			}

			return details;
		}
		void createSwapChain(VkDevice device, VkPhysicalDevice physical_device, VkSurfaceKHR surface, VkSwapchainKHR* swapchain) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physical_device, surface);

			VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
			VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
			VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

			uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
			if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
				imageCount = swapChainSupport.capabilities.maxImageCount;
			}

			VkSwapchainCreateInfoKHR createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfo.surface = surface;

			createInfo.minImageCount = imageCount;
			createInfo.imageFormat = surfaceFormat.format;
			createInfo.imageColorSpace = surfaceFormat.colorSpace;
			createInfo.imageExtent = extent;
			createInfo.imageArrayLayers = 1;
			createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

			QueueFamilyIndices indices = vk::utils::device::findQueueFamilies(physical_device, surface);
			uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

			if (indices.graphicsFamily != indices.presentFamily) {
				createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfo.queueFamilyIndexCount = 2;
				createInfo.pQueueFamilyIndices = queueFamilyIndices;
			}
			else {
				createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			}

			createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
			createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfo.presentMode = presentMode;
			createInfo.clipped = VK_TRUE;

			if (vkCreateSwapchainKHR(device, &createInfo, nullptr, swapchain) != VK_SUCCESS) {
				throw std::runtime_error("failed to create swap chain!");
			}

			vkGetSwapchainImagesKHR(device, *swapchain, &imageCount, nullptr);
			swapChainImages.resize(imageCount);
			vkGetSwapchainImagesKHR(device, *swapchain, &imageCount, swapChainImages.data());

			swapChainImageFormat = surfaceFormat.format;
			swapChainExtent = extent;
		}
	}

	namespace command {
		void createCommandPool(VkCommandPool* command_pool, VkDevice device, VkPhysicalDevice gpu, VkSurfaceKHR surface) {
			QueueFamilyIndices queueFamilyIndices = vk::utils::device::findQueueFamilies(gpu, surface);

			VkCommandPoolCreateInfo create_info = vk::utils::initializer::getCommandPoolCreateInfo();
			create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			create_info.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

			if (vkCreateCommandPool(device, &create_info, nullptr, command_pool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create graphics command pool!");
			}
		}
	}

	namespace shader {

		std::vector<char> readFile(const std::string& filename) {
			std::ifstream file(filename, std::ios::ate | std::ios::binary);

			if (!file.is_open()) {
				throw std::runtime_error("failed to open file!");
			}

			size_t fileSize = (size_t)file.tellg();
			std::vector<char> buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);

			file.close();

			return buffer;
		}

		VkShaderModule createShaderModule(VkDevice device, const std::string& filename) {
			std::vector<char> code = readFile(filename);

			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = code.size();
			createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

			VkShaderModule shaderModule;
			if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
				throw std::runtime_error("failed to create shader module!");
			}

			return shaderModule;
		}

		VkPipelineShaderStageCreateInfo createShaderStageCreateInfo(VkShaderModule shader_module, VkShaderStageFlagBits stage) {
			VkPipelineShaderStageCreateInfo shader_stage_create_info{};
			shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shader_stage_create_info.stage = stage;
			shader_stage_create_info.module = shader_module;
			shader_stage_create_info.pName = "main";
			return shader_stage_create_info;
		}
	}

}
