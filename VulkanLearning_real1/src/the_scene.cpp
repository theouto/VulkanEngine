#include "../include/the_scene.hpp"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <memory>


namespace lve
{

  LveScene::LveScene(const char *file, LveGameObject::Map &objects, LveDevice &device, 
                     std::vector<std::unique_ptr<LveDescriptorSetLayout>> layouts, LveDescriptorPool &pool)
                    : lveDevice{device}, filepath{file}, gameObjects{objects}
  {
    materialHandler = std::make_unique<LveMaterials>(lveDevice);
    load(layouts, pool);
  }

  void LveScene::load(std::vector<std::unique_ptr<LveDescriptorSetLayout>> sceneLayouts, LveDescriptorPool &pool)
  {
    std::ifstream scene(filepath);
    if (!scene.is_open()) {throw std::runtime_error("Failed to open scene file!");}

    std::shared_ptr<LveModel> lveModel = nullptr;
    std::string line, model, material;
    glm::vec3 rotation{}, scale{}, translation{};
    getline(scene, line);

    int i = 0;
    while (line[0] != EOF)
    {
      model = line;
      getline(scene, line);
      material = line;
      scene >> rotation.x >> rotation.y >> rotation.z;
      scene >> scale.x >> scale.y >> scale.x;
      scene >> translation.x >> translation.y >> translation.z;

      lveModel = LveModel::createModelFromFile(lveDevice, model);

      objArr.push_back(LveGameObject::createGameObject());
      objArr[i].textures = materialHandler->retrieveMaterial(material);
      objArr[i].transform.translation = translation;
      objArr[i].transform.rotation = rotation;
      objArr[i].transform.scale = scale;

      objArr[i].write_material(*sceneLayouts[1], *sceneLayouts[2],pool);
      gameObjects.emplace(objArr[i].getId(), std::move(objArr[i]));
      i++;
      getline(scene, line);
    }

    scene.close();
  }
}
