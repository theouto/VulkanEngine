#include "../include/the_scene.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <memory>


namespace lve
{

  LveScene::LveScene(std::string file, LveGameObject::Map &objects, LveDevice *device, 
                     LveDescriptorSetLayout& sceneLayouts, LveDescriptorSetLayout& normalLayout, LveDescriptorPool& pool)
                    : lveDevice{device}, filepath{file.c_str()}, gameObjects{objects}
  {
    materialHandler = std::make_unique<LveMaterials>(*lveDevice);
    load(sceneLayouts, normalLayout, pool);
  }

  void LveScene::load(LveDescriptorSetLayout& sceneLayout, LveDescriptorSetLayout& normalLayout,LveDescriptorPool &pool)
  {
    std::ifstream scene(filepath);
    if (!scene.is_open()) {throw std::runtime_error("Failed to open scene file!");}

    std::shared_ptr<LveModel> lveModel = nullptr;
    std::string line, model, material;
    glm::vec3 rotation{}, scale{}, translation{};
    getline(scene, line);

    int i = 0;

      model = line;
      getline(scene, line);
      material = line;

      scene >> rotation.x >> rotation.y >> rotation.z;
      scene >> scale.x >> scale.y >> scale.x;
      scene >> translation.x >> translation.y >> translation.z;

      lveModel = LveModel::createModelFromFile(*lveDevice, model);
      auto object = LveGameObject::createGameObject();

      object.textures = materialHandler->retrieveMaterial(material);
      object.transform.translation = translation;
      object.transform.rotation = rotation;
      object.transform.scale = scale;

      object.write_material(sceneLayout, normalLayout, pool);

      gameObjects.emplace(object.getId(), std::move(object));
      i++;
      getline(scene, line);

      std::cout << "times " << i << '\n';

    scene.close();
  }
}
