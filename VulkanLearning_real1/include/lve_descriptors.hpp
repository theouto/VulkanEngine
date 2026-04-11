#pragma once

#include "lve_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace lve {

    class LveDescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(LveDevice& lveDevice) : lveDevice{ lveDevice } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);

            Builder& addBindingFlag(
                     VkDescriptorBindingFlags bindingFlags,
                     uint32_t count = 1);

            std::unique_ptr<LveDescriptorSetLayout> build() const;

        private:
            LveDevice& lveDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
            std::vector<VkDescriptorBindingFlags> bindingFlags{};
        };

        LveDescriptorSetLayout(
            LveDevice& lveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~LveDescriptorSetLayout(){}
        LveDescriptorSetLayout(const LveDescriptorSetLayout&) = delete;
        LveDescriptorSetLayout& operator=(const LveDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
        void createLayoutTextures();

    private:
        LveDevice& lveDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
        std::vector<VkDescriptorBindingFlags> bindingFlags;

        friend class LveDescriptorWriter;
    };

    class LveDescriptorPool {
    public:
        class Builder {
        public:
            Builder(LveDevice& lveDevice) : lveDevice{ lveDevice } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<LveDescriptorPool> build() const;

        private:
            LveDevice& lveDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            std::vector<std::pair<VkDescriptorType, uint32_t>> placehold;
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        LveDescriptorPool(
            LveDevice& lveDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes,
            std::vector<std::pair<VkDescriptorType, uint32_t>> placehold);
        ~LveDescriptorPool();
        LveDescriptorPool(const LveDescriptorPool&) = delete;
        LveDescriptorPool& operator=(const LveDescriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor);

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;
        bool expandPool(const VkDescriptorPoolCreateInfo &poolInfo);
        VkDescriptorPool getDescriptorPool() {return descriptorPool;}
        void resetPool();

    private:
        LveDevice& lveDevice;
        VkDescriptorPool descriptorPool;
        uint32_t maxSets;
        VkDescriptorPoolCreateFlags poolFlags;
        std::vector<std::pair<VkDescriptorType, uint32_t>> sizes;

        friend class LveDescriptorWriter;
    };

    class LveDescriptorWriter {
    public:
        LveDescriptorWriter(LveDescriptorSetLayout& setLayout, LveDescriptorPool& pool);

        LveDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        LveDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);
        LveDescriptorWriter& addImage(uint32_t binding, VkDescriptorImageInfo* imageInfo, uint32_t index);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        LveDescriptorSetLayout& setLayout;
        LveDescriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}  // namespace lve
