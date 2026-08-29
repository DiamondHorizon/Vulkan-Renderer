#include "vulkan_window.hpp"

// std
#include <stdexcept>

namespace vulkan {
	// Constructor
	VulkanWindow::VulkanWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}

	// Deconstructor
	VulkanWindow::~VulkanWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void VulkanWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr); // Initalize the window pointer
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void VulkanWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface");
		}
	}

	void VulkanWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto vulkanWindow = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
		vulkanWindow->framebufferResized = true;
		vulkanWindow->width = width;
		vulkanWindow->height = height;
	}

	void VulkanWindow::toggleFullscreen(GLFWwindow* window) {
		static bool isFullscreen = false;
		static int windowedWidth = 1280;
		static int windowedHeight = 720;
		static int windowPosX = 100;
		static int windowPosY = 100;

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		if (!isFullscreen) {
			glfwGetWindowPos(window, &windowPosX, &windowPosY);
			glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
			glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
			isFullscreen = true;
		}
		else {
			glfwSetWindowMonitor(window, nullptr, windowPosX, windowPosY, windowedWidth, windowedHeight, 0);
			isFullscreen = false;
		}
	}
}