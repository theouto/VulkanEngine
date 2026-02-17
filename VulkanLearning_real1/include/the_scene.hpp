#pragma once

#include <vector>

#include "lve_game_object.hpp"
#include "lve_device.hpp"
#include "the_materials.hpp"

namespace lve
{
  class LveScene
  {
    public:

      LveScene(const char* file, LveGameObject::Map &objects, LveDevice &device, 
               std::vector<std::unique_ptr<LveDescriptorSetLayout>> sceneLayouts,
               LveDescriptorPool &pool);

    private:

      void load(std::vector<std::unique_ptr<LveDescriptorSetLayout>> sceneLayouts, LveDescriptorPool &pool);

      std::unique_ptr<LveMaterials> materialHandler;
      std::vector<LveGameObject> objArr;
      LveDevice &lveDevice;
      const char* filepath;
      LveGameObject::Map &gameObjects;
  };
}
