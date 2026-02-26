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

      LveScene(LveDevice &device, LveGameObject::Map& objects);

      void load(std::string file, LveDescriptorPool& pool);

      void loadModel(LveGameObject& object, LveDescriptorPool& pool, const char* path);
      LveDescriptorSetLayout& mattLayout(){return *matLayout;}
      LveDescriptorSetLayout& normallLayout(){return *normalLayout;}

    private:

      LveDevice& lveDevice;

      std::unique_ptr<LveDescriptorSetLayout> matLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //albedo
            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //specular
            .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //normal
            .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //roughness
            .addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //AO
            .addBinding(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //metalness
            .build();

      std::unique_ptr<LveDescriptorSetLayout> normalLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();


      std::unique_ptr<LveMaterials> materialHandler;
      std::vector<LveGameObject> objArr;
      LveGameObject::Map& gameObjects;
    };
}
