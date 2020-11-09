#include "utils.h"

void vk::utils::checkResult(VkResult result) {
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed");
	}
}
