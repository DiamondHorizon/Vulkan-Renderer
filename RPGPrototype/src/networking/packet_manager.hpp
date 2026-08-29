#pragma once

// Libs
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace networking {
    enum PacketType : uint8_t {
        CHAT = 0,
        PLAYER_SPAWN = 1,
        MOVEMENT_INPUT = 2,
        CUSTOM = 3,
        // Add more types as needed
    };

    struct PacketHeader {
        PacketType type;
        uint16_t length; // Length of payload
    };

    struct Packet {
        PacketHeader header;
        std::vector<uint8_t> payload;
    };

    struct MovementInput {
        uint32_t frameID = 0;   // Frame number this input was captured on
        uint8_t moveBits = 0;  // 8 bits: W/A/S/D/Up/Down/Jump etc.
        uint8_t lookBits = 0;  // Optional — for rotational input or camera mode
    };

    struct PlayerSpawnPacket {
        float posX, posY, posZ;     // Initial spawn position
    };
    
    class PacketManager {
    public:
        static std::vector<uint8_t> serialize(const Packet& packet);
        static Packet parsePacket(const uint8_t* data, size_t length);

        static Packet createChatPacket(const std::string& message);
        static Packet createPlayerSpawnPacket(const glm::vec3& position);
        static Packet createMovementInputPacket(void* peerId, const MovementInput& input);
    };
}