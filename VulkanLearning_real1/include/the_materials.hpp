#pragma once

#include "lve_textures.hpp"
#include "lve_device.hpp"

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

    static VkDescriptorSet write_material(std::vector<std::shared_ptr<LveTextures>> textures,
                                          LveDescriptorSetLayout& descLayout, 
                                          LveDescriptorPool& descPool);

    private:

    LveDevice &lveDevice;
    std::unordered_map<unsigned int, 
                       std::pair<VkDescriptorSet, 
                       std::vector<std::shared_ptr<LveTextures>>>> loadedMaterials;
  };
}
