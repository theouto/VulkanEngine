#pragma once

#include "lve_textures.hpp"
#include "lve_device.hpp"

#include <memory>
#include <vector>
#include <map>
#include <string>
#include <vulkan/vulkan_core.h>

namespace lve
{
  class LveMaterials
  {
    public:

    LveMaterials(LveDevice &device);
    ~LveMaterials(){}

    std::vector<VkDescriptorSet> retrieveMaterial(const std::string path,
                                            LveDescriptorSetLayout& descLayout,
                                            LveDescriptorSetLayout& normalLayout,
                                            LveDescriptorPool& descPool);

    private:
    
    LveDevice &lveDevice;
    std::vector<std::shared_ptr<LveTextures>> textures;
    std::unordered_map<std::string, std::pair<std::vector<VkDescriptorSet>, //I swear, this is for the greater good
                                    std::vector<std::shared_ptr<LveTextures>>>> loadedMaterials;
  };
}
