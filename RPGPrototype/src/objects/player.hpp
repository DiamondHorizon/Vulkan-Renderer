#pragma once

#include "entity.hpp"

#include "../input_controller.hpp"
#include "../vulkan_model.hpp"

// std
#include <array>

namespace objects {
    class Player : public Entity {
    public:
        enum class Role {
            Local,
            Remote
        };
        Role role = Role::Local;
        void* peerId = nullptr;
        bool inputEnabled = true;
        float moveSpeed{ 3.f };
        float lookSpeed{ 1.f };
        networking::MovementInput lastMovementInput;

        explicit Player(id_t objId, Role r) : Entity(objId), role(r) {}

        static std::shared_ptr<Player> createPlayer(Role role, std::shared_ptr<vulkan::VulkanModel> model) {
            static id_t currentId = 2000;
            auto p = std::make_shared<Player>(currentId++, role);
            p->model = model;
            p->transform.scale = { 0.5f, 1.f, 0.5f };
            return p;
        }

        void update(float deltaTime) override {
            // Movement, animation, etc.
        }

        void syncFromNetwork(const networking::Packet& packet) override {
            // Apply movement input or position
        }

        void applyInput(const networking::MovementInput& input, float dt) {
            float yaw = transform.rotation.y;
            const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
            const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
            const glm::vec3 upDir{ 0.f, -1.f, 0.f };

            glm::vec3 moveDir{ 0.f };
            if (input.moveBits & vulkan::InputController::MOVE_FORWARD)  moveDir += forwardDir;
            if (input.moveBits & vulkan::InputController::MOVE_BACKWARD) moveDir -= forwardDir;
            if (input.moveBits & vulkan::InputController::MOVE_LEFT)     moveDir += rightDir;   // Assuming right is Z-axis inverted
            if (input.moveBits & vulkan::InputController::MOVE_RIGHT)    moveDir -= rightDir;
            if (input.moveBits & vulkan::InputController::MOVE_UP)       moveDir += upDir;
            if (input.moveBits & vulkan::InputController::MOVE_DOWN)     moveDir -= upDir;

            if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
                transform.translation += moveSpeed * dt * glm::normalize(moveDir);
            }

            if (input.moveBits & vulkan::InputController::JUMP) {
                // Trigger jump animation or vertical velocity here
            }

            // Interpret lookBits to rotate player
            glm::vec3 lookDelta{ 0 };
            if (input.lookBits & vulkan::InputController::LOOK_RIGHT)   lookDelta.y -= 1.f;
            if (input.lookBits & vulkan::InputController::LOOK_LEFT)    lookDelta.y += 1.f;
            if (input.lookBits & vulkan::InputController::LOOK_UP)      lookDelta.x += 1.f;
            if (input.lookBits & vulkan::InputController::LOOK_DOWN)    lookDelta.x -= 1.f;

            if (glm::dot(lookDelta, lookDelta) > std::numeric_limits<float>::epsilon()) {
                transform.rotation += lookSpeed * dt * glm::normalize(lookDelta);
            }

            // Limit pitch values between about +/- 85ish degrees
            transform.rotation.x = glm::clamp(transform.rotation.x, -1.5f, 1.5f);
            transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());
        }
    };
}