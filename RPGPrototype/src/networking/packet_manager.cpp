#include "packet_manager.hpp"

#include "../input_controller.hpp"

// std
#include <iostream>

namespace networking {
    // Converts the structured packet object into raw bytes
    std::vector<uint8_t> PacketManager::serialize(const Packet& packet) {
        std::vector<uint8_t> buffer;

        // Serialize header
        buffer.push_back(static_cast<uint8_t>(packet.header.type));

        uint16_t len = packet.header.length;
        buffer.push_back(static_cast<uint8_t>(len & 0xFF));         // low byte
        buffer.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));  // high byte

        // Append payload
        buffer.insert(buffer.end(), packet.payload.begin(), packet.payload.end());

        return buffer;
    }

    // Converts the raw bytes into a structured packet object
    Packet PacketManager::parsePacket(const uint8_t* data, size_t length) {
        Packet packet;

        if (length < 3) {
            std::cerr << "[PacketManager] Invalid packet: too short.\n";
            return packet; // Return empty/default packet
        }

        // Parse header
        packet.header.type = static_cast<PacketType>(data[0]);

        uint16_t payloadLength = static_cast<uint16_t>(data[1]) |
            (static_cast<uint16_t>(data[2]) << 8);
        packet.header.length = payloadLength;

        // Validate payload length
        if (length < 3 + payloadLength) {
            std::cerr << "[PacketManager] Invalid packet: declared length exceeds buffer.\n";
            return packet;
        }

        // Copy payload
        packet.payload = std::vector<uint8_t>(data + 3, data + 3 + payloadLength);

        return packet;
    }

    Packet PacketManager::createChatPacket(const std::string& message) {
        Packet packet;
        packet.header.type = PacketType::CHAT;
        packet.payload = std::vector<uint8_t>(message.begin(), message.end());
        packet.header.length = static_cast<uint16_t>(packet.payload.size());
        return packet;
    }

    Packet PacketManager::createPlayerSpawnPacket(const glm::vec3& position) {
        PlayerSpawnPacket spawn{};
        spawn.posX = position.x;
        spawn.posY = position.y;
        spawn.posZ = position.z;

        Packet packet;
        packet.header.type = PacketType::PLAYER_SPAWN;
        packet.payload.resize(sizeof(spawn));
        std::memcpy(packet.payload.data(), &spawn, sizeof(spawn));
        packet.header.length = static_cast<uint16_t>(packet.payload.size());
        return packet;
    }

    Packet PacketManager::createMovementInputPacket(void* peerId, const MovementInput& input) {
        Packet packet;
        packet.header.type = PacketType::MOVEMENT_INPUT;    
        packet.payload.resize(sizeof(peerId) + sizeof(MovementInput));
        std::memcpy(packet.payload.data(), &peerId, sizeof(peerId));
        std::memcpy(packet.payload.data(), &input, sizeof(MovementInput));
        packet.header.length = static_cast<uint16_t>(packet.payload.size());
        return packet;
    }
}
