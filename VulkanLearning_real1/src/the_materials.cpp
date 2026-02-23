#include "../include/the_materials.hpp"
#include "../include/lve_game_object.hpp"

#include <memory>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace lve
{
  LveMaterials::LveMaterials(LveDevice& device) : lveDevice{device} {}

  std::vector<VkDescriptorSet> LveMaterials::retrieveMaterial(
                                            const std::string path,
                                            LveDescriptorSetLayout& descLayout,
                                            LveDescriptorSetLayout& normalLayout,
                                            LveDescriptorPool& descPool)
  {
    std::ifstream material(path);
    if (!material.is_open()) {throw std::runtime_error("Failed to open material file!");}

    std::string dummy;
    getline(material, dummy);
    //std::vector<std::shared_ptr<LveTextures>> materials;
    std::pair<std::vector<VkDescriptorSet>, std::vector<std::shared_ptr<LveTextures>>> materials;
    try {materials.first = loadedMaterials.at(dummy).first;} catch (std::out_of_range e)
    {
      std::string mat_id = dummy;
      textures.resize(6);
      for (int i = 0; i < 6; i++)
      {
        getline(material, dummy);
        LveTextures::texType format = LveTextures::SINGLE_UNORM;
        if (i == 0) format = LveTextures::COLOR;
        else if (i == 2) format = LveTextures::NORMAL;

        textures[i] = std::make_shared<LveTextures>(lveDevice, dummy, format);
      }

      std::cout << "emplacing textures...\n";

      materials.second = textures;
      materials.first = LveGameObject::write_material(descLayout, materials.second,
                                                      normalLayout, descPool);
      
      loadedMaterials.emplace(mat_id, materials);

      std::cout << "emplaced!\n";
    };

    return materials.first;
  }
}
