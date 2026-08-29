#pragma once

#include "vulkan_descriptors.hpp"
#include "vulkan_device.hpp"
#include "objects/game_object.hpp"
#include "vulkan_window.hpp"
#include "vulkan_renderer.hpp"

#include "networking/networking.hpp"

#include "objects/player.hpp"

// std
#include <memory>
#include <thread>
#include <vector>

namespace vulkan {
	extern networking::Networking networkManager;

	class App {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		static bool networkingConnected;
		static std::unordered_map<unsigned int, std::shared_ptr<objects::Player>> players;
		static std::unordered_map<uint32_t, std::shared_ptr<objects::GameObject>> gameObjects;

		App();
		~App();

		// Prevent copying App so there are not mulitple pointers to the App
		App(const App&) = delete;
		App& operator=(const App&) = delete;

		void run();

		void spawnPlayer(const networking::PlayerSpawnPacket& spawn, void* peer);

	private:
		static inline std::thread networkThread;
		static inline bool networkingThreadRunning = false;

		void loadGameObjects();
		void checkNetworkMessages();
		void processNetworkEvents();

		VulkanWindow vulkanWindow{ WIDTH, HEIGHT, "Vulkan Window" };
		VulkanDevice vulkanDevice{ vulkanWindow };
		VulkanRenderer vulkanRenderer{ vulkanWindow, vulkanDevice };

		std::unique_ptr<VulkanDescriptorPool> globalPool{};
		objects::Player* localPlayer = nullptr; // Raw pointer used to keep ownership in gameObjects
	};
}

// Rendering
// TODO: Sort files into folders as shown here: https://youtu.be/Z1lLwAEMt4M?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&t=700
// TODO: Implment different scenes (or rooms)
// TODO: Draw all point lights together: https://youtu.be/1olS6ayckKM?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&t=65
// TODO: Make the program run on several different (all) OSs and consoles
// https://youtu.be/uZqxj6tLDY4?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&t=3

// Networking
// TODO: Use EOS Lobbies to Discover Players

// Extra
// TODO: Make item that causes setDistortionView() to replace setViewDirection() in run() of app.cpp