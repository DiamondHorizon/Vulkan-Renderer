#pragma once

#include "packet_manager.hpp"

#include "../objects/player.hpp"

// std
#include <array>
#include <cstdint>
#include <mutex>
#include <queue>
#include <string>

namespace networking {
    enum class NetworkEventType {
        HostConnected,
        HostDisconnected,
        PeerConnected,
        PeerConnectedBroadcast,
        PeerDisconnected
    };

    struct NetworkEvent {
        NetworkEventType type;
        void* peer;
        Packet* packet = nullptr;
    };

    extern std::unordered_map<void*, uint32_t> peerToObjectId;
    extern std::unordered_map<uint32_t, void*> objectIdToPeer;
    inline std::queue<std::string> incomingMessages;
    inline std::mutex messageMutex;
    inline std::queue<NetworkEvent> pendingNetworkEvents;
    inline std::mutex eventMutex;

    class Networking {
    public:

        Networking();

        bool isHost = false;
        void* hostPeer = nullptr;

        bool createHost(bool asServer, uint16_t port = 55555);
        bool connectToPeer(const char* ip, uint16_t port);
        void pollIncomingPackets();
        static objects::Player* getPlayerFromPeer(void* peer);
        static void sendReliable(void* targetPeer, const Packet& packet);
        static void sendUnreliable(void* targetPeer, const Packet& packet);
        void sendChatMessage(const std::string & text);

    private:
        void initNetworking();
        void shutdownNetworking();
    };
}