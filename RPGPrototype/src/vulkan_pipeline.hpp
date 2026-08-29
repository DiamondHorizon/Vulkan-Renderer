#pragma once

#include "vulkan_device.hpp"

// std
#include <string>
#include <vector>

namespace vulkan {
	struct PipelineConfigInfo {
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class VulkanPipeline {
		public:
			// Constructor
			VulkanPipeline(VulkanDevice &device, const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo);
			// Deconstructor
			~VulkanPipeline();

			// Delete copy constructors to prevent duplicating pointers to Vulkan objects
			VulkanPipeline(const VulkanPipeline&) = delete;
			VulkanPipeline& operator = (const VulkanPipeline&) = delete;

			void bind(VkCommandBuffer commandBuffer);

			static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
			static void enableAlphaBlending(PipelineConfigInfo& configInfo);

		private:
			static std::vector<char> readFile(const std::string& filepath);

			void createGraphicsPipeline(const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo);

			void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

			VulkanDevice& vulkanDevice;
			VkPipeline graphicsPipeline;
			VkShaderModule vertShaderModule;
			VkShaderModule fragShaderModule;
	};
}