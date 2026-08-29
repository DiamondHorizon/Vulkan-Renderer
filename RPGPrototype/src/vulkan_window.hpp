#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace vulkan {
	
	class VulkanWindow {
		public:
			VulkanWindow(int w, int h, std::string name);
			~VulkanWindow();

			// Prevent copying VulkanWindow so there are not mulitple pointers to the GLFW window
			VulkanWindow(const VulkanWindow&) = delete;
			VulkanWindow& operator=(const VulkanWindow&) = delete;

			bool shouldClose() { return glfwWindowShouldClose(window); }; // Check if user has tried to dismiss the window and return a boolean value as an indication
			VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
			bool wasWindowResized() { return framebufferResized; }
			void resetWindowResizedFlag() { framebufferResized = false; }
			GLFWwindow* getGLFWwindow() const { return window; }

			static void toggleFullscreen(GLFWwindow* window);

			void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		private:
			static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
			void initWindow();

			int width;
			int height;
			bool framebufferResized = false;

			std::string windowName;
			GLFWwindow* window;
	};
}