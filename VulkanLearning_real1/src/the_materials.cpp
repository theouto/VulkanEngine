#include "../xxHash/xxhash.h"

#include "../include/the_materials.hpp"

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

  VkDescriptorSet LveMaterials::retrieveMaterial(const std::string path, 
                                                 LveDescriptorSetLayout& descLayout,
                                                 LveDescriptorPool& descPool)
  {
    std::ifstream material(path);
    if (!material.is_open()) {throw std::runtime_error("Failed to open material file!");}
    XXH32_hash_t hash = XXH32(path.c_str(), path.length(), 0);

    std::cout << "path: " << path.c_str() << '\n' << "hash: " << hash << '\n';

    std::string dummy;
    VkDescriptorSet load{};
    try {load = loadedMaterials.at(hash).first;} catch (std::out_of_range e)
    {
      std::pair<VkDescriptorSet, std::vector<std::shared_ptr<LveTextures>>> materials;
      std::string mat_id = dummy;
      for (int i = 0; i < 6; i++)
      {
        getline(material, dummy);
        LveTextures::texType format = LveTextures::SINGLE_UNORM;
        if (i == 0) format = LveTextures::COLOR;
        else if (i == 2) format = LveTextures::NORMAL;

        materials.second.push_back(std::make_shared<LveTextures>(lveDevice, dummy, format));
      }

      materials.first = write_material(materials.second, descLayout, descPool);
      load = materials.first;
      std::cout << "emplacing textures...\n";

      loadedMaterials.emplace(hash, materials);

      std::cout << "emplaced!\n";
    };

    return load;
  }

    VkDescriptorSet LveMaterials::write_material(std::vector<std::shared_ptr<LveTextures>> textures,
                                                  LveDescriptorSetLayout& descLayout,
                                                  LveDescriptorPool& descPool)
    {
      VkDescriptorSet descriptor{};

      auto colorInfo = textures[0]->getDescriptorInfo();
      auto specInfo = textures[1]->getDescriptorInfo();
      auto normInfo = textures[2]->getDescriptorInfo();
      auto dispInfo = textures[3]->getDescriptorInfo();
      auto AOInfo = textures[4]->getDescriptorInfo();
      auto metalInfo = textures[5]->getDescriptorInfo();

      LveDescriptorWriter(descLayout, descPool)
                .writeImage(1, &colorInfo) // colour
                .writeImage(2, &specInfo) //spec 
                .writeImage(3, &normInfo) //normal
                .writeImage(4, &dispInfo) //displacement
                .writeImage(5, &AOInfo)
                .writeImage(6, &metalInfo)
                .build(descriptor);

      return descriptor;
    }
}
