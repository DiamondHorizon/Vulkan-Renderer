#pragma once

#include "vulkan_camera.hpp"
#include "objects/game_object.hpp"

// lib
#include <vulkan/vulkan.h>

namespace vulkan {

	#define MAX_LIGHTS 10 // TODO: Optomize to make this so it can be increased without an issue

	struct PointLight {
		glm::vec3 position{};
		alignas(16) glm::vec4 color{}; // w is intensity
	};

	struct GlobalUbo {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::mat4 inverseView{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f };
		PointLight pointLights[MAX_LIGHTS];
		int numLights;
	};

	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		VulkanCamera& camera;
		VkDescriptorSet globalDescriptorSet;
		objects::GameObject::Map& gameObjects;
	};
}