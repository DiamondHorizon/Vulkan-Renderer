#pragma once

#include "vulkan_device.hpp"
#include "vulkan_model.hpp"
#include "vulkan_swap_chain.hpp"
#include "vulkan_window.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace vulkan {
	class VulkanRenderer {
	public:
		VulkanRenderer(VulkanWindow &window, VulkanDevice &device);
		~VulkanRenderer();

		// Prevent copying VulkanRenderer so there are not mulitple pointers to the VulkanRenderer
		VulkanRenderer(const VulkanRenderer&) = delete;
		VulkanRenderer& operator=(const VulkanRenderer&) = delete;

		// Accessor functions
		VkRenderPass getSwapChainRenderPass() const { return vulkanSwapChain->getRenderPass(); }
		float getAspectRatio() const { return vulkanSwapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return isFrameStarted; }
		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot get command buffer when frame is not in progress");
			return commandBuffers[currentFrameIndex];
		}
		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		VulkanWindow& vulkanWindow;
		VulkanDevice& vulkanDevice;
		std::unique_ptr <VulkanSwapChain> vulkanSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex;
		bool isFrameStarted;
	};
}