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
	class PointLightSystem {
	public:
		PointLightSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~PointLightSystem();

		// Prevent copying PointLightSystem so there are not mulitple pointers to the PointLightSystem
		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator=(const PointLightSystem&) = delete;

		void update(FrameInfo& frameInfo, GlobalUbo& ubo);
		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		VulkanDevice& vulkanDevice;
		std::unique_ptr<VulkanPipeline> vulkanPipeline;
		VkPipelineLayout pipelineLayout;
	};
}