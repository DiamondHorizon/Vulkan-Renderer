#include "app.hpp"

#include "input_controller.hpp"
#include "vulkan_buffer.hpp"
#include "vulkan_camera.hpp"

#include "music/music.hpp"

#include "networking/networking.hpp"
#include "networking/packet_manager.hpp"

#include "objects/player.hpp"

#include "systems/vulkan_render_system.hpp"
#include "systems/point_light_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <chrono>
#include <cassert>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>

namespace vulkan {
	networking::Networking networkManager;
	bool App::networkingConnected = false;
	std::unordered_map<unsigned int, std::shared_ptr<objects::Player>> vulkan::App::players;
	std::unordered_map<uint32_t, std::shared_ptr<objects::GameObject>> App::gameObjects;

	App::App() {
		globalPool = VulkanDescriptorPool::Builder(vulkanDevice)
			.setMaxSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VulkanSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
		
		loadGameObjects();
	}

	App::~App() {
		if (networkingThreadRunning) {
			networkingThreadRunning = false;  // Signal the thread to exit
			if (networkThread.joinable()) {
				networkThread.join();         // Wait for the thread to finish
			}
		}
		globalPool = nullptr;
	}

	void App::run() {
        // Constants
        constexpr float MAX_FRAME_TIME = 0.25f; // 250ms

		// Play music
		music::Music musicPlayer;
		musicPlayer.play("Invasive_Species_Invasant_Theme");

		// Uniform Buffer
		std::vector<std::unique_ptr<VulkanBuffer>> uboBuffers(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(vulkanDevice.getPhysicalDevice(), &properties);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<VulkanBuffer>(vulkanDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, properties.limits.minUniformBufferOffsetAlignment);
			uboBuffers[i]->map();
		}

		auto globalSetLayout = VulkanDescriptorSetLayout::Builder(vulkanDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			VkDescriptorBufferInfo bufferInfo = uboBuffers[i]->descriptorInfo();
			VulkanDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}
		
		VulkanRenderSystem vulkanRenderSystem{ vulkanDevice, vulkanRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
		PointLightSystem pointLightSystem{ vulkanDevice, vulkanRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        VulkanCamera camera{};
        camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

		// Move the camera
		auto viewerObject = objects::GameObject::createGameObject();
		viewerObject->transform.translation.z = -2.5f;
		
		// Move the player obj with the camera
		glm::vec3 forward = camera.getFrontVector();
		glm::vec3 offset = glm::vec3(0.f, -0.5f, 0.f); // Adjust height to offset the player object

		localPlayer->transform.translation = viewerObject->transform.translation + forward * 0.5f + offset;
		localPlayer->transform.rotation = viewerObject->transform.rotation;

		auto currentTime = std::chrono::high_resolution_clock::now();

		// Get keyboard input
        InputController controller{};


		while (!vulkanWindow.shouldClose()) { // While the window doesn't want to close...
			glfwPollEvents(); // Take keystrokes or mouse interactions
            // TODO: Make window resizing smoother on other platforms https://youtu.be/0IIqvi3Z0ng?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&t=512

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            frameTime = glm::min(frameTime, MAX_FRAME_TIME);

			// Offset in local space (behind and above the player)
			glm::vec3 offset = glm::vec3(0.f, 2.f, 5.f);

			// TODO: add collision checks or minimum distance from the player to keep the camera from clipping through geometry.

			// Rotate offset based on player's yaw (Y-axis rotation)
			localPlayer->transform.rotation.x = glm::clamp(localPlayer->transform.rotation.x, -1.1f, 1.1f);			
			float pitch = localPlayer->transform.rotation.x;
			float yaw = localPlayer->transform.rotation.y;

			// Compute full rotation from yaw and pitch
			glm::mat4 rotateYaw = glm::rotate(glm::mat4(1.f), yaw, glm::vec3(0.f, 1.f, 0.f));  // Y-axis
			glm::mat4 rotatePitch = glm::rotate(glm::mat4(1.f), pitch, glm::vec3(1.f, 0.f, 0.f));  // X-axis
			glm::mat4 rotation = rotateYaw * rotatePitch;
			
			glm::vec3 rotatedOffset = glm::vec3(rotation * glm::vec4(offset, 1.f));

			// Final camera position
			glm::vec3 camPos = localPlayer->transform.translation - rotatedOffset;

			// Set view toward the player
			camera.setViewTarget(camPos, localPlayer->transform.translation);

			// Get player input
			networking::MovementInput input = controller.collectMovementInput(vulkanWindow.getGLFWwindow(), static_cast<uint32_t>(frameTime));
			if (localPlayer->inputEnabled) {
				localPlayer->applyInput(input, frameTime);
			}
			if (networkingConnected) {
				if (networkManager.isHost) {
					for (auto& [id, player] : vulkan::App::players) {
						networkManager.sendUnreliable(networking::objectIdToPeer[id], networking::PacketManager::createMovementInputPacket(localPlayer->peerId, input));
					}
				} else {
					networkManager.sendUnreliable(networkManager.hostPeer, networking::PacketManager::createMovementInputPacket(localPlayer->peerId, input));
				}
			}
			for (auto& [id, player] : vulkan::App::players) {
				if (player->role == objects::Player::Role::Remote && player->inputEnabled) {
					player->applyInput(player->lastMovementInput, frameTime);
				}
			}

			// TODO: Merge methods
			controller.checkFullscreenToggle(vulkanWindow.getGLFWwindow()); // Check for toggling fullscreen
			controller.getUserInputs(vulkanWindow.getGLFWwindow());


			viewerObject->transform.translation = localPlayer->transform.translation;  // Sync camera to player

			// Networking
			if (!networkingThreadRunning && networkingConnected) {
				networkingThreadRunning = true;
				networkThread = std::thread([] {
					while (networkingThreadRunning) {
						networkManager.pollIncomingPackets();
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
					}
				});
			}
			checkNetworkMessages();
			processNetworkEvents();

			// Update players
			for (const auto& [id, player] : players) {
				player->update(frameTime);
			}

            float aspect = vulkanRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f); // Last value is render distance, TODO: Make this changeable in the settings
			
			VkCommandBuffer commandBuffer = vulkanRenderer.beginFrame();
			if (commandBuffer == nullptr) {
				continue;  // Skip rendering this frame
			}
			int frameIndex = vulkanRenderer.getFrameIndex();
			FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects};

			// Update
			GlobalUbo ubo{};
			ubo.projection = camera.getProjection();
			ubo.view = camera.getView();
			ubo.inverseView = camera.getInverseView();
			pointLightSystem.update(frameInfo, ubo);
			uboBuffers[frameIndex]->writeToBuffer(&ubo, sizeof(GlobalUbo));
			uboBuffers[frameIndex]->flush();

			// TODO: (Next 3 lines)
			// Begin offscreen shadow pass
			// Render shadow casting objects
			// End offscreen shadow pass

			// Render
			vulkanRenderer.beginSwapChainRenderPass(commandBuffer);

			// Opaque objects before semi-transparent ones
			vulkanRenderSystem.renderGameObjects(frameInfo);
			pointLightSystem.render(frameInfo);

			vulkanRenderer.endSwapChainRenderPass(commandBuffer);
			vulkanRenderer.endFrame();
		}
		vkDeviceWaitIdle(vulkanDevice.device());
	}

	void App::loadGameObjects() {
		// Instructions for exporting model from Blender: https://youtu.be/wfh2N4u-nOU?list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&t=814
        std::shared_ptr<VulkanModel> vulkanModel = VulkanModel::createModelFromFile(vulkanDevice, "models/cube.obj");

		auto localPlayerObj = objects::Player::createPlayer(objects::Player::Role::Local, vulkanModel);
		localPlayerObj->transform.translation = { 0.f, 0.f, 0.f };
		localPlayerObj->transform.scale = { 0.5f, 1.f, 0.5f };
		localPlayer = localPlayerObj.get(); // Optional pointer for easy reference

		players.emplace(localPlayerObj->getId(), localPlayerObj);      // Add to players container
		gameObjects.emplace(localPlayerObj->getId(), localPlayerObj);  // Add to renderable objects

		vulkanModel = VulkanModel::createModelFromFile(vulkanDevice, "models/flat_vase.obj");

        auto flatVase = objects::GameObject::createGameObject();
		flatVase->model = vulkanModel;
		flatVase->transform.translation = { -.5f, .5f, 0.f };
		flatVase->transform.scale = { 3.f, 1.5f, 3.f };
        gameObjects.emplace(flatVase->getId(), std::move(flatVase));

		vulkanModel = VulkanModel::createModelFromFile(vulkanDevice, "models/smooth_vase.obj");

		auto smoothVase = objects::GameObject::createGameObject();
		smoothVase->model = vulkanModel;
		smoothVase->transform.translation = { .5f, .5f, 0.f };
		smoothVase->transform.scale = { 3.f, 1.5f, 3.f };
		gameObjects.emplace(smoothVase->getId(), std::move(smoothVase));

		vulkanModel = VulkanModel::createModelFromFile(vulkanDevice, "models/quad.obj");

		auto floor = objects::GameObject::createGameObject();
		floor->model = vulkanModel;
		floor->transform.translation = { 0.f, .5f, 0.f };
		floor->transform.scale = { 3.f, 1.f, 3.f };
		gameObjects.emplace(floor->getId(), std::move(floor));

		std::vector<glm::vec3> lightColors{
			{1.f, .1f, .1f},
			{.1f, .1f, 1.f},
			{.1f, 1.f, .1f},
			{1.f, 1.f, .1f},
			{.1f, 1.f, 1.f},
			{1.f, 1.f, 1.f} // TODO: Make black lighting
		};

		for (int i = 0; i < lightColors.size(); i++) {
			auto pointLight = objects::GameObject::makePointLight(0.2f);
			pointLight->color = lightColors[i];
			auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(), { 0.f, -1.f, 0.f });
			pointLight->transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, -1.f));
			gameObjects.emplace(pointLight->getId(), std::move(pointLight));
		}
	}

	void App::checkNetworkMessages() {
		std::lock_guard<std::mutex> msgLock(networking::messageMutex);

		while (!networking::incomingMessages.empty()) {
			std::string msg = networking::incomingMessages.front();
			networking::incomingMessages.pop();

			// Handle the message however you want:
			std::cout << "[Main Thread] Received message: " << msg << "\n";

			// Optionally route it by type, trigger events, etc.
			if (msg == "pong") {
				// Trigger UI feedback or game logic
				// TODO: Display messages in chat window - TODO: Make chat window
			}
		}
	}

	void App::processNetworkEvents() {
		std::lock_guard<std::mutex> lock(networking::eventMutex);

		while (!networking::pendingNetworkEvents.empty()) {
			networking::NetworkEvent event = networking::pendingNetworkEvents.front();
			networking::pendingNetworkEvents.pop();

			switch (event.type) {
				case networking::NetworkEventType::HostConnected: { // Client connected to host (Client side)
					// Send information about local player
					networking::Packet spawnPacket = networking::PacketManager::createPlayerSpawnPacket(localPlayer->transform.translation);
					networking::Networking::sendReliable(event.peer, spawnPacket);
					break;
				}
				case networking::NetworkEventType::HostDisconnected: {
					// TODO
					break;
				}
				case networking::NetworkEventType::PeerConnected: { // Host accepted new peer (Host and Client Side)
					// Create new player when another client connects
					networking::PlayerSpawnPacket spawn;
					std::memcpy(&spawn, event.packet->payload.data(), sizeof(spawn));
					spawnPlayer(spawn, event.peer);

					// Send info about all existing players to the new peer
					// TODO: FIX!!!
					/*for (const auto& [existingId, existingPlayer] : players) {
						if (existingId == networking::peerToPlayerId[event.peer]) continue;

						networking::Packet otherPlayerPacket = networking::PacketManager::createPlayerSpawnPacket(existingId, existingPlayer->transform.translation);
						networking::Networking::sendReliable(event.peer, otherPlayerPacket);
					}*/
					break;
				}
				case networking::NetworkEventType::PeerConnectedBroadcast: {
					// Send info about all existing players to the new peer
					for (const auto& [existingId, existingPlayer] : players) {
						if (existingId == networking::peerToObjectId[event.peer]) continue; // Skip the new player

						networking::Packet otherPlayerPacket = networking::PacketManager::createPlayerSpawnPacket(existingPlayer->transform.translation);
						networking::Networking::sendReliable(event.peer, otherPlayerPacket);
					}
					break;
				}
				case networking::NetworkEventType::PeerDisconnected: {
					//networking::removeRemotePlayer(evt.peer); TODO
					break;
				}
			}
		}
	}

	void App::spawnPlayer(const networking::PlayerSpawnPacket& spawn, void* peer) {
		std::shared_ptr<VulkanModel> vulkanModel = VulkanModel::createModelFromFile(vulkanDevice, "models/cube.obj"); // TODO: Make this dynamic
		if (!vulkanModel) {
			std::cerr << "[App] Failed to load model for spawn.\n";
			return;
		}

		auto remotePlayer = objects::Player::createPlayer(objects::Player::Role::Remote, vulkanModel);
 		remotePlayer->transform.translation = { spawn.posX, spawn.posY, spawn.posZ };
		remotePlayer->transform.scale = { 0.5f, 1.f, 0.5f };
		remotePlayer->peerId = peer; // Assign the Peer Id to the player object

		// Add player to object lists
		players.emplace(remotePlayer->getId(), remotePlayer);
		gameObjects.emplace(remotePlayer->getId(), remotePlayer);

		// Add player and Id to maps
		networking::peerToObjectId[peer] = remotePlayer->getId(); 
		networking::objectIdToPeer[remotePlayer->getId()] = peer;

		std::cout << "[App] Spawned remote player with object ID " << remotePlayer->getId() << " at " << spawn.posX << ", " << spawn.posY << ", " << spawn.posZ << "\n";
	}
}