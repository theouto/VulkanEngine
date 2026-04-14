#pragma once

#include "lve_textures.hpp"
#include "lve_device.hpp"

#include <unordered_map>
#include <vector>
#include <map>
#include <string>

namespace lve
{
  class LveMaterials
  {
    public:

    LveMaterials(LveDevice &device);
    ~LveMaterials(){}

    VkDescriptorSet retrieveMaterial(const std::string path,
                                     LveDescriptorSetLayout& descLayout,
                                     LveDescriptorPool& descPool);

    std::vector<uint32_t> retrieveBindless(const std::string path,
                                           LveDescriptorSetLayout& descLayout,
                                           LveDescriptorPool& descPool,
                                           VkDescriptorSet& bindlessSet);

    static VkDescriptorSet writeMaterial(std::vector<std::shared_ptr<LveTextures>> textures,
                                          LveDescriptorSetLayout& descLayout, 
                                          LveDescriptorPool& descPool);

    void writeBindless(std::vector<std::shared_ptr<LveTextures>> textures,
                               LveDescriptorSetLayout& descLayout, 
                               LveDescriptorPool& descPool,
                               VkDescriptorSet& bindlessSet);

    //this will be removed once the bindless descriptors stop being an experiment, but for nowwwww yeahhhhhhhhh
    static std::shared_ptr<LveTextures> write_test(LveDevice& lveDevice);

    private:

    uint32_t currArr = 1;

    std::vector<std::shared_ptr<LveTextures>> totalTextures;

    LveDevice &lveDevice;
    std::unordered_map<unsigned int,
                       std::pair<VkDescriptorSet,
                       std::vector<std::shared_ptr<LveTextures>>>> loadedMaterials;

    std::unordered_map<unsigned int,
                       std::vector<uint32_t>> bindlessTextureSet;

    //The point here is to trade memory for less writes
    std::unordered_map<unsigned int,
                       uint32_t> textures;
  };
}
