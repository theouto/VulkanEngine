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

      materials.first = writeMaterial(materials.second, descLayout, descPool);
      load = materials.first;
      std::cout << "emplacing textures...\n";

      loadedMaterials.emplace(hash, materials);

      std::cout << "emplaced!\n";
    };

    return load;
  }

  std::vector<uint32_t> LveMaterials::retrieveBindless(const std::string path,
                                                       LveDescriptorSetLayout& descLayout,
                                                       LveDescriptorPool& descPool,
                                                       VkDescriptorSet& bindlessSet)
  {
    std::ifstream material(path);
    if (!material.is_open()) {throw std::runtime_error("Failed to open material file!");}
    XXH32_hash_t hash = XXH32(path.c_str(), path.length(), 0);

    std::string dummy;
    std::vector<uint32_t> load(6, 0);
    try {load = bindlessTextureSet.at(hash);} catch (std::out_of_range e)
    {
      std::vector<std::shared_ptr<LveTextures>> toWrite;
      std::string mat_id = dummy;
      for (int i = 0; i < 6; i++)
      {
        getline(material, dummy);
        try {load.push_back(textures.at(XXH32(dummy.c_str(), dummy.length(), 0)));} catch (std::out_of_range e)
        {
          LveTextures::texType format = LveTextures::SINGLE_UNORM;
          if (i == 0) format = LveTextures::COLOR;
          else if (i == 2) format = LveTextures::NORMAL;

          auto tex = std::make_shared<LveTextures>(lveDevice, dummy, format);

          toWrite.push_back(tex);
          totalTextures.push_back(tex);
          load[i] = ++currArr;
        }
      }

      writeBindless(toWrite, descLayout, descPool, bindlessSet);

      for(int i = 0; i < load.size(); i++)
      {
        std::cout << '\n' << load[i] << '\n';
      }

      std::cout << "emplacing bindless texture set...\n";

      bindlessTextureSet.emplace(hash, load);

      std::cout << "emplaced!\n";
    };

    return load;
  }

  VkDescriptorSet LveMaterials::writeMaterial(std::vector<std::shared_ptr<LveTextures>> textures,
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
              .writeImage(1, &colorInfo)
              .writeImage(2, &specInfo)
              .writeImage(3, &normInfo)
              .writeImage(4, &dispInfo)
              .writeImage(5, &AOInfo)
              .writeImage(6, &metalInfo)
              .build(descriptor);

    return descriptor;
  }

  void LveMaterials::writeBindless(std::vector<std::shared_ptr<LveTextures>> textures,
                                    LveDescriptorSetLayout& descLayout, 
                                    LveDescriptorPool& descPool,
                                    VkDescriptorSet& bindlessSet)
  {
    for (int i = 0; i < textures.size(); i++)
    {
      auto texInfo = textures[i]->getDescriptorInfo();

      LveDescriptorWriter(descLayout, descPool)
                .addImage(0, &texInfo, currArr - (textures.size() - i - 1))
                .overwrite(bindlessSet);

      std::cout << currArr - textures.size() - i - 1 << '\n';
    }
  }

  std::shared_ptr<LveTextures> LveMaterials::write_test(LveDevice& lveDevice)
  {
    return std::make_shared<LveTextures>(lveDevice, "textures/NA.png",LveTextures::COLOR);
  }
}
