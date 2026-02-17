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

    std::vector<std::shared_ptr<LveTextures>> retrieveMaterial(const std::string path);

    private:
    
    LveDevice &lveDevice;
    std::map<std::string, std::vector<std::shared_ptr<LveTextures>>> loadedMaterials;
  };
}
