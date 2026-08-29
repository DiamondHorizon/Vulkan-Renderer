#include "input_controller.hpp"

#include "app.hpp"

#include "vulkan_window.hpp"

#include "networking/networking.hpp"
#include "networking/packet_manager.hpp"

// libs
#include <winsock2.h>
#include <ws2tcpip.h>

// std
#include <limits>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

namespace vulkan {
	networking::MovementInput InputController::collectMovementInput(GLFWwindow* window, uint32_t frameTime) {
		networking::MovementInput input;
		input.frameID = frameTime;

		if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
			// TODO: Implement controller input
			GLFWgamepadstate state;
			if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state)) {
				if (state.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS) {
					std::cout << "A button pressed\n";
				}

				float leftX = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
				float leftY = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];

				if (leftY < -0.2f) input.moveBits |= MOVE_FORWARD;   // Stick up
				if (leftY > 0.2f) input.moveBits |= MOVE_BACKWARD;  // Stick down
				if (leftX < -0.2f) input.moveBits |= MOVE_LEFT;      // Stick left
				if (leftX > 0.2f) input.moveBits |= MOVE_RIGHT;     // Stick right
			}
		} else {
			// Movement
			if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) input.moveBits |= MOVE_FORWARD;
			if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) input.moveBits |= MOVE_BACKWARD;
			if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) input.moveBits |= MOVE_LEFT;
			if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) input.moveBits |= MOVE_RIGHT;
			if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) input.moveBits |= MOVE_UP;
			if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) input.moveBits |= MOVE_DOWN;
			if (glfwGetKey(window, keys.jump) == GLFW_PRESS) input.moveBits |= JUMP;
			if (glfwGetKey(window, keys.crouch) == GLFW_PRESS) input.moveBits |= CROUCH;

			// Looking
			if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) input.lookBits |= LOOK_LEFT;
			if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) input.lookBits |= LOOK_RIGHT;
			if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) input.lookBits |= LOOK_UP;
			if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) input.lookBits |= LOOK_DOWN;

		}
		return input;
	}

	// TODO: Merge with next method
	void InputController::checkFullscreenToggle(GLFWwindow* window) {
		static bool toggledThisFrame = false;

		bool altPressed = glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
			glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS;
		bool enterPressed = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;

		if (altPressed && enterPressed && !toggledThisFrame) {
			VulkanWindow::toggleFullscreen(window);
			toggledThisFrame = true;
		}
		else if (!(altPressed && enterPressed)) {
			toggledThisFrame = false;
		}
	}

	// TODO: Merge with previous method
	void InputController::getUserInputs(GLFWwindow* window) {
		bool isDown = glfwGetKey(window, keys.chat) == GLFW_PRESS;
		bool wasDown = keyWasDownLastFrame[keys.chat];

		if (isDown && !wasDown) {
			networkManager.sendChatMessage("ping");
		}
		if (glfwGetKey(window, keys.host) == GLFW_PRESS && !keyWasDownLastFrame[keys.host]) {
			networkManager.createHost(true);
			keyWasDownLastFrame[keys.host] = GLFW_PRESS;
		}
		if (glfwGetKey(window, keys.client) == GLFW_PRESS && !keyWasDownLastFrame[keys.client]) {
			networkManager.createHost(false);
			keyWasDownLastFrame[keys.client] = GLFW_PRESS;
		}
		if (glfwGetKey(window, keys.connect) == GLFW_PRESS && !keyWasDownLastFrame[keys.connect]) {
			networkManager.connectToPeer(getLocalIPAddress().c_str(), 55555);
			keyWasDownLastFrame[keys.connect] = GLFW_PRESS;
		}


		keyWasDownLastFrame[keys.chat] = isDown;
	}

	// TEMP: TODO: REMOVE
	std::string InputController::getLocalIPAddress() {
		WSADATA wsaData;
		char hostname[256];
		std::string ipStr = "Unavailable";

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0) {
			if (gethostname(hostname, sizeof(hostname)) == 0) {
				addrinfo hints{}, * res = nullptr;
				hints.ai_family = AF_INET; // Use AF_UNSPEC for IPv4 + IPv6
				hints.ai_socktype = SOCK_STREAM;

				if (getaddrinfo(hostname, nullptr, &hints, &res) == 0) {
					for (addrinfo* p = res; p != nullptr; p = p->ai_next) {
						sockaddr_in* ipv4 = reinterpret_cast<sockaddr_in*>(p->ai_addr);
						char ip[INET_ADDRSTRLEN];
						inet_ntop(AF_INET, &(ipv4->sin_addr), ip, sizeof(ip));
						ipStr = ip;
						break; // Grab the first one
					}
					freeaddrinfo(res);
				}
			}
			WSACleanup();
		}

		return ipStr;
	}
}