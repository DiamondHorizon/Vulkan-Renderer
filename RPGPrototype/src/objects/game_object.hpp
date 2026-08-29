#pragma once

#include "../vulkan_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace objects {
	struct TransformComponent {
		glm::vec3 translation{}; // Positionn offset
		glm::vec3 scale = { 1.f, 1.f, 1.f }; // Scale offset
		glm::vec3 rotation{}; // Rotation offset

		// TODO: Figure out faster and more reliable rotation rendering - https://youtu.be/0X_kRtyVzm4?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&t=547
		
		// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3) - This order is intrinsically rotating when read form left to right and extrinsically rotating when read from right to left
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 mat4();

		glm::mat3 normalMatrix();
	};

	struct PointLightComponent {
		float lightIntensity = 1.0f;
	};
	
	class GameObject {
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, std::shared_ptr<GameObject>>;

		explicit GameObject(id_t objId) : id{ objId } {}

		static std::shared_ptr<GameObject> createGameObject() {
			static id_t currentId = 0;
			return std::make_shared<GameObject>(currentId++);
		}

		static std::shared_ptr<GameObject> makePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

		GameObject(const GameObject&) = delete;
		GameObject& operator=(const GameObject&) = delete;
		GameObject(GameObject&&) = default;
		GameObject& operator=(GameObject&&) = default;

		const id_t getId() { return id; }

		glm::vec3 color{};
		TransformComponent transform{};

		// Optional components
		std::shared_ptr<vulkan::VulkanModel> model{};
		std::unique_ptr<PointLightComponent> pointLight = nullptr;

	private:
		id_t id;
	};
}