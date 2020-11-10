#include "Device.h"

#include <stdexcept>
#include <set>
#include <string>

// --------------- static ----------------------

static bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> device_extensions) {
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

bool Device::isDeviceSuitable(VkPhysicalDevice device) {
	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device, this->device_extensions);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

// ------------------ public ------------------
Device::Device(VkInstance* instance) {
	if (instance == nullptr) {
		throw std::runtime_error("gived VkInstance* is nullptr");
	}
	this->instance = instance;
}

void Device::selectPhysicalDevice(bool(*isSuitable)(VkPhysicalDevice gpu)) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(*instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(*instance, &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (isSuitable == nullptr) {
			if (isDeviceSuitable(device)) {
				this->physical_device = device;
				sample_count_flag_bits = getMaxUsableSampleCount();
				break;
			}
		}
		else {
			if (isSuitable(device)) {
				this->physical_device = device;
				sample_count_flag_bits = getMaxUsableSampleCount();
				break;
			}
		}
	}

	if (this->physical_device == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}



void Device::setDeviceQueues(VkSurfaceKHR* surface)const {

}

// --------------------- private ------------------------

VkSampleCountFlagBits Device::getMaxUsableSampleCount() {
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(this->physical_device, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}
