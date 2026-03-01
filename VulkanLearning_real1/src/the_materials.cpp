#include "../xxHash/xxhash.h"

#include "../include/the_materials.hpp"

#include <memory>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <memory>

namespace lve
{
  LveMaterials::LveMaterials(LveDevice& device) : lveDevice{device} {}

  std::vector<std::shared_ptr<LveTextures>> LveMaterials::retrieveMaterial(const std::string path)
  {
    std::ifstream material(path);
    if (!material.is_open()) {throw std::runtime_error("Failed to open material file!");}
    XXH32_hash_t hash = XXH32(path.c_str(), 256, 0);

    std::string dummy;
    std::vector<std::shared_ptr<LveTextures>> materials;
    try {materials = loadedMaterials.at(hash);} catch (std::out_of_range e)
    {
      std::string mat_id = dummy;
      for (int i = 0; i < 6; i++)
      {
        getline(material, dummy);
        LveTextures::texType format = LveTextures::SINGLE_UNORM;
        if (i == 0) format = LveTextures::COLOR;
        else if (i == 2) format = LveTextures::NORMAL;

        materials.push_back(std::make_shared<LveTextures>(lveDevice, dummy, format));
      }

      std::cout << "emplacing textures...\n";

      loadedMaterials.emplace(hash, materials);

      std::cout << "emplaced!\n";
    };

    return materials;
  }
}
