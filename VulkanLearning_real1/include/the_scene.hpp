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

      LveScene(LveDevice &device);

      //Dodgy workaround until I work it out
      void load(std::string file, LveDescriptorSetLayout& sceneLayout, LveDescriptorSetLayout& normalLayout, 
                LveDescriptorPool& pool, LveGameObject::Map &objects);

    private:

      std::unique_ptr<LveMaterials> materialHandler;
      std::vector<LveGameObject> objArr;
      LveDevice& lveDevice;
  };
}
