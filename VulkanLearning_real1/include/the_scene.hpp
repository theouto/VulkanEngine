#pragma once

#include <vector>

#include "lve_descriptors.hpp"
#include "lve_game_object.hpp"
#include "lve_device.hpp"
#include "the_materials.hpp"

namespace lve
{
  class LveScene
  {
    public:

      LveScene(std::string file, LveGameObject::Map &objects, LveDevice *device, 
               LveDescriptorSetLayout& sceneLayouts, LveDescriptorSetLayout& normalLayout,
               LveDescriptorPool &pool);

    private:

      void load(LveDescriptorSetLayout& sceneLayout, LveDescriptorSetLayout& normalLayout,LveDescriptorPool& pool);

      std::unique_ptr<LveMaterials> materialHandler;
      std::vector<LveGameObject> objArr;
      LveDevice *lveDevice;
      const char* filepath;
      LveGameObject::Map &gameObjects;
  };
}
