#pragma once

#include "vulkan_window.hpp"

#include "networking/packet_manager.hpp"

#include "objects/game_object.hpp"

namespace vulkan {
	class InputController {
	public:
		enum MovementFlags : uint8_t {
			MOVE_FORWARD = 1 << 0,		// W
			MOVE_BACKWARD = 1 << 1,		// S
			MOVE_LEFT = 1 << 2,			// A
			MOVE_RIGHT = 1 << 3,		// D
			MOVE_UP = 1 << 4,			// Q
			MOVE_DOWN = 1 << 5,			// E
			JUMP = 1 << 6,				// SPACE
			CROUCH = 1 << 7				// LEFT CONTROL
		};

		enum LookFlags : uint8_t {
			LOOK_LEFT = 1 << 0,			// LEFT ARROW
			LOOK_RIGHT = 1 << 1,		// RIGHT ARROW
			LOOK_UP = 1 << 2,			// UP ARROW
			LOOK_DOWN = 1 << 3,			// DOWN ARROW
		};

		struct KeyMappings {
			int moveLeft = GLFW_KEY_A;
			int moveRight = GLFW_KEY_D;
			int moveForward = GLFW_KEY_W;
			int moveBackward = GLFW_KEY_S;
			int moveUp = GLFW_KEY_Q;
			int moveDown = GLFW_KEY_E;
			int jump = GLFW_KEY_SPACE;
			int crouch = GLFW_KEY_LEFT_CONTROL;
			int lookLeft = GLFW_KEY_LEFT;
			int lookRight = GLFW_KEY_RIGHT;
			int lookUp = GLFW_KEY_UP;
			int lookDown = GLFW_KEY_DOWN;
			int chat = GLFW_KEY_C;
			int host = GLFW_KEY_H;
			int client = GLFW_KEY_J;
			int connect = GLFW_KEY_K;
		};

		networking::MovementInput collectMovementInput(GLFWwindow* window, uint32_t frameTime);
		void checkFullscreenToggle(GLFWwindow* window);
		void getUserInputs(GLFWwindow* window);

		KeyMappings keys{};

	private:
		std::unordered_map<int, bool> keyWasDownLastFrame;
		std::string getLocalIPAddress();
	};
}