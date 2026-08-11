#pragma once

#include <vector>

#include "lve_renderer.hpp"
#include "lve_descriptors.hpp"
#include "lve_game_object.hpp"
#include "lve_device.hpp"
#include "the_materials.hpp"

namespace lve
{
  class LveScene
  {
    public:

      LveScene(LveDevice &device, LveGameObject::Map& objects, LveRenderer& renderer);

      void load(std::string file, LveDescriptorPool& pool);
      void loadInstanced(std::string file, LveDescriptorPool& pool); //non-destructive testing
      void saveScene();

      void createPointLightHelper(std::ifstream& scene);
      void createObjectHelper(std::ifstream& scene, LveDescriptorPool& pool);

      void loadModel(LveGameObject& object, LveDescriptorPool& pool, 
                           LveDescriptorPool& bindlessPool, 
                           LveDescriptorSetLayout& bindlessLayout, 
                           VkDescriptorSet& bindlessSet,
                           const char* path);

      void changeMaterial(LveGameObject& object,
                                LveDescriptorPool& bindlessPool, 
                                LveDescriptorSetLayout& bindlessLayout, 
                                VkDescriptorSet& bindlessSet,
                                const char* path);

      uint32_t retrieveModel(XXH32_hash_t hash, std::string model);

      LveMaterials& handler() {return *materialHandler;}
      //LveDescriptorSetLayout& mattLayout(){return *matLayout;}

    private:

      LveDevice& lveDevice;

      /*
      std::unique_ptr<LveDescriptorSetLayout> matLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //albedo
            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //specular
            .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //normal
            .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //roughness
            .addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //AO
            .addBinding(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //metalness
            .build();
      */

      std::string line, model, material, name;
      float intensity, radius;
      glm::vec3 rotation{}, scale{1.f, 1.f, 1.f}, translation{}, color{};
      int count, type;
      std::shared_ptr<LveModel> lveModel = nullptr;

      std::string currentScene;

      std::unordered_map<uint32_t, std::shared_ptr<LveModel>> models;
      std::unique_ptr<LveMaterials> materialHandler;
      std::vector<LveGameObject> objArr;
      LveGameObject::Map& gameObjects;
      LveRenderer& lveRenderer;
    };
}
