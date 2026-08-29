#pragma once

#include "game_object.hpp"

#include "../vulkan_model.hpp"

#include "../networking/packet_manager.hpp"

// std
#include <iostream>

namespace objects {
    class Entity : public GameObject {
    public:
        // Gameplay-relevant fields
        std::string name;
        bool isAlive = true;

        explicit Entity(id_t objId) : GameObject(objId) {}

        static std::shared_ptr<Entity> createEntity(...) {
            static id_t currentId = 1000;
            return std::make_shared<Entity>(currentId++);
        }

        virtual void update(float frameTime) {}
        virtual void syncFromNetwork(const networking::Packet& packet) {}
    };
}