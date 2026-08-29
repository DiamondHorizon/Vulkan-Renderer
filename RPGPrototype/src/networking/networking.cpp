#include "networking.hpp"

#include "packet_manager.hpp"

#include "../app.hpp"
#include "../input_controller.hpp"

// libs
#define NOMINMAX

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

#define ENET_IMPLEMENTATION

// Force IPv4 socket creation before enet.h pulls in its logic
#define ENET_SOCKET_CREATE(type) socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)
#define ENET_SOCKET_BIND(socket, address) bind(socket, (struct sockaddr*) address, sizeof(struct sockaddr_in))

#include "enet/include/enet.h"

// std
#include <iostream>
#include <mutex>

namespace networking {
    std::unordered_map<void*, uint32_t> peerToObjectId;
    std::unordered_map<uint32_t, void*> objectIdToPeer;
    inline std::mutex networkingMutex;
    static ENetHost* host = nullptr;

    Networking::Networking() {
        initNetworking();
    }

    void Networking::initNetworking() {
        if (enet_initialize() != 0) {
            std::cerr << "[ENet] Failed to initialize.\n";
        }
    }

    void Networking::shutdownNetworking() {
        enet_deinitialize();
        std::cout << "[ENet] Shut down.\n";
    }

    bool Networking::createHost(bool asHost, uint16_t port) {
        isHost = asHost; // Record where session is as host or as client

        ENetAddress address;

        if (asHost) {
            // Set address to listen on all IPv6 interfaces (::)
            enet_address_set_host_new(&address, "::");
            address.port = port;
        }

        std::lock_guard<std::mutex> lock(networkingMutex);
        host = enet_host_create(
            asHost ? &address : nullptr,    // Server binds, client passes nullptr
            asHost ? 32 : 1,                // Max connections
            2,                                // Channels
            0,                                // Incoming bandwidth
            0                                 // Outgoing bandwidth
        );

        if (!host) {
            std::cerr << "[ENet] Failed to create " << (asHost ? "host" : "client") << ".\n";
            return false;
        }

        char boundIP[256];
        enet_address_get_host_ip(&host->address, boundIP, sizeof(boundIP));
        std::cout << "[ENet] Session successfully created as " << (asHost ? "host" : "client") << " at " << boundIP << ":" << host->address.port << "\n";

        if (asHost) {
            ENetPeer* peer = enet_host_connect(host, &address, 2, 0);
            hostPeer = static_cast<void*>(peer);
        }

        vulkan::App::networkingConnected = true; 
        return true;
    }

    bool Networking::connectToPeer(const char* ip, uint16_t port) {
        ENetAddress address;

        if (!host) {
            std::cerr << "[ENet] Host is null — cannot connect.\n";
            return false;
        }

        // Resolve target IP — accepts IPv6 or IPv4 strings (e.g. ::ffff:192.168.x.x)
        if (enet_address_set_host_new(&address, ip) != 0) {
            std::cerr << "[ENet] Failed to resolve IP: " << ip << "\n";
            return false;
        }

        address.port = port;

        //char resolvedIP[256];
        //enet_address_get_host_ip(&address, resolvedIP, sizeof(resolvedIP));
        //std::cout << "[ENet] Attempting to connect to peer at " << resolvedIP << ":" << port << "\n";

        std::lock_guard<std::mutex> lock(networkingMutex);
        ENetPeer* peer = enet_host_connect(host, &address, 2, 0);
        if (!peer) {
            std::cerr << "[ENet] Failed to initiate peer connection.\n";
            return false;
        }

        //std::cout << "[ENet] Connection attempt sent to " << resolvedIP << ":" << port << "\n";
        return true;
    }

    void Networking::pollIncomingPackets() {
        std::lock_guard<std::mutex> lock(networkingMutex);
        ENetEvent event;
        while (enet_host_service(host, &event, 5) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    std::cout << "[ENet] Connected to peer.\n";

                    // Trigger additional player creation and setup
                    std::lock_guard<std::mutex> lock(eventMutex);
                    if (!isHost) {
                        hostPeer = static_cast<void*>(event.peer);
                        pendingNetworkEvents.push({ NetworkEventType::HostConnected, static_cast<void*>(event.peer) });
                    }
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    void* disconnectedPeer = static_cast<void*>(event.peer);
                    auto it = networking::peerToObjectId.find(disconnectedPeer);
                    if (it != networking::peerToObjectId.end()) {
                        uint32_t playerId = it->second;
                        vulkan::App::players.erase(playerId);
                        vulkan::App::gameObjects.erase(playerId);
                        networking::peerToObjectId.erase(disconnectedPeer);
                    }
                    std::cout << "[ENet] Peer disconnected.\n";
                    break;
                }
                case ENET_EVENT_TYPE_RECEIVE: {
                    Packet parsed = PacketManager::parsePacket(event.packet->data, event.packet->dataLength);

                    switch (parsed.header.type) {
                        case PacketType::CHAT: {
                            std::string msg(parsed.payload.begin(), parsed.payload.end());
                            std::cout << "[Chat] " << msg << "\n";
                            std::lock_guard<std::mutex> msgLock(messageMutex);
                            incomingMessages.push(msg); // Raw string, or serialize a packet if needed
                            break;
                        }
                        case PacketType::PLAYER_SPAWN: { // Recieved by the host, sent from client
                            if (parsed.payload.size() < sizeof(networking::PlayerSpawnPacket)) {
                                std::cerr << "[Network] Spawn packet too small.\n";
                                break;
                            }

                            if (isHost) {
                                for (const auto& [peer, playerId] : peerToObjectId) {
                                    if (peer == event.peer) continue; // Skip original sender
                                    networking::Networking::sendReliable(peer, parsed);
                                }
                            }

                            std::lock_guard<std::mutex> lock(eventMutex);
                            networking::Packet* packetPtr = new networking::Packet(parsed);
                            pendingNetworkEvents.push({ NetworkEventType::PeerConnected, static_cast<void*>(event.peer), static_cast<Packet*>(packetPtr) });
                            if (isHost) pendingNetworkEvents.push({ NetworkEventType::PeerConnectedBroadcast, static_cast<void*>(event.peer) });
                            break;
                        }
                        case PacketType::MOVEMENT_INPUT: {
                            if (parsed.payload.size() < sizeof(uint32_t) + sizeof(MovementInput)) {
                                // Log error or discard packet
                                break;
                            }

                            void* peerId;
                            MovementInput input;
                            std::memcpy(&peerId, parsed.payload.data(), sizeof(peerId));
                            std::memcpy(&input, parsed.payload.data() + sizeof(peerId), sizeof(MovementInput));
                            std::cerr << "[Debug] Received peerId: " << peerId << "\n";

                            auto movedPlayer = getPlayerFromPeer(peerId);
                            if (movedPlayer) {
                                std::cerr << "[Error] here.\n";
                                movedPlayer->lastMovementInput = input;
                            }
                            else {
                                std::cerr << "[Error] NOT here.\n";
                            }
                            break;
                        }
                        case PacketType::CUSTOM: {
                            // Handle other types
                            std::cout << "Custom packet type received.\n";
                            break;
                        }
                        default:
                            std::cout << "[Warning] Unknown packet type.\n";
                            break;
                        }

                    enet_packet_destroy(event.packet);
                    break;
                }
            }
        }
    }

    objects::Player* Networking::getPlayerFromPeer(void* peer) {
        auto itId = peerToObjectId.find(peer);
        if (itId == peerToObjectId.end()) return nullptr;

        auto itPlayer = vulkan::App::players.find(itId->second);
        if (itPlayer == vulkan::App::players.end()) return nullptr;

        return itPlayer->second.get();
    }

    void Networking::sendReliable(void* recipient, const Packet &packet) {
        ENetPeer* targetPeer = static_cast<ENetPeer*>(recipient);

        if (!targetPeer || !host) {
            std::cerr << "[Error] sendReliable: peer or host is null.\n";
            return;
        }

        std::vector<uint8_t> raw = PacketManager::serialize(packet);
        ENetPacket* enetPacket = enet_packet_create(raw.data(), raw.size(), ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(targetPeer, 0, enetPacket);
        enet_host_flush(host);
    }

    void Networking::sendUnreliable(void* recipient, const Packet &packet) {
        ENetPeer* targetPeer = static_cast<ENetPeer*>(recipient);

        if (!targetPeer || !host) {
            //std::cerr << "[Error] sendUnreliable: peer or host is null.\n";
            return;
        }

        std::vector<uint8_t> raw = PacketManager::serialize(packet);
        ENetPacket* enetPacket = enet_packet_create(raw.data(), raw.size(), 0);
        enet_peer_send(targetPeer, 0, enetPacket);
    }

    void Networking::sendChatMessage(const std::string& text) {
        /*std::lock_guard<std::mutex> lock(networkingMutex);

        if (peer != nullptr && host != nullptr) {
            Packet chatPacket = PacketManager::createChatPacket(text);
            sendReliable(static_cast<void*>(peer), chatPacket);
        }
        else {
            std::cout << "[ENet] No Peer Found\n";
        }*/
        return;
    }
}