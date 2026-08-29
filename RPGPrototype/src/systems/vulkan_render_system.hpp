#pragma once

#include "../vulkan_camera.hpp"
#include "../vulkan_device.hpp"
#include "../vulkan_frame_info.hpp"
#include "../objects/game_object.hpp"
#include "../vulkan_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace vulkan {
	class VulkanRenderSystem {
	public:
		VulkanRenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~VulkanRenderSystem();

		// Prevent copying VulkanRenderSystem so there are not mulitple pointers to the VulkanRenderSystem
		VulkanRenderSystem(const VulkanRenderSystem&) = delete;
		VulkanRenderSystem& operator=(const VulkanRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		VulkanDevice& vulkanDevice;
		std::unique_ptr<VulkanPipeline> vulkanPipeline;
		VkPipelineLayout pipelineLayout;
	};
}