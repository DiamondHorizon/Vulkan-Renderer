#pragma once

#include "vulkan_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace vulkan {

    class VulkanDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(VulkanDevice& vulkanDevice) : vulkanDevice{ vulkanDevice } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<VulkanDescriptorSetLayout> build() const;

        private:
            VulkanDevice& vulkanDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        VulkanDescriptorSetLayout(
            VulkanDevice& vulkanDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~VulkanDescriptorSetLayout();
        VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout&) = delete;
        VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        VulkanDevice& vulkanDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class VulkanDescriptorWriter;
    };

    class VulkanDescriptorPool {
    public:
        class Builder {
        public:
            Builder(VulkanDevice& vulkanDevice) : vulkanDevice{ vulkanDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count); // Declare how many descriptors to execpt
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<VulkanDescriptorPool> build() const;

        private:
            VulkanDevice& vulkanDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        VulkanDescriptorPool(
            VulkanDevice& vulkanDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~VulkanDescriptorPool();
        VulkanDescriptorPool(const VulkanDescriptorPool&) = delete;
        VulkanDescriptorPool& operator=(const VulkanDescriptorPool&) = delete;


        // Allocate and free descriptors from the pool
        bool allocateDescriptorSet(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        // Reset the descriptors in the pool
        void resetPool();

    private:
        VulkanDevice& vulkanDevice;
        VkDescriptorPool descriptorPool;

        friend class VulkanDescriptorWriter;
    };

    class VulkanDescriptorWriter {
    public:
        VulkanDescriptorWriter(VulkanDescriptorSetLayout& setLayout, VulkanDescriptorPool& pool);

        VulkanDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        VulkanDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        VulkanDescriptorSetLayout& setLayout;
        VulkanDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}