#pragma once

#include <memory>
#include <unordered_map>
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
      void createObject(std::string name, std::string model,
                              std::string material, glm::vec3 translation,
                              glm::vec3 scale, glm::vec3 rotation,
                              LveDescriptorPool& pool);

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
      std::unordered_map<uint32_t, std::shared_ptr<LveModel>>& modelMap() {return models;}
      std::shared_ptr<LveModel> getActiveModel() {return lveModel;}

      LveMaterials& handler() {return *materialHandler;}

    private:

      LveDevice& lveDevice;

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
