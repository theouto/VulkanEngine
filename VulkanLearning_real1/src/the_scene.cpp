#include "../include/the_scene.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <memory>


namespace lve
{

  LveScene::LveScene(LveDevice &device, LveGameObject::Map& objects) : lveDevice{device}, gameObjects{objects}
  {
    materialHandler = std::make_unique<LveMaterials>(lveDevice);
  }

  void LveScene::load(std::string file, LveDescriptorPool& pool)
  {
    std::ifstream scene(file.c_str());
    if (!scene.is_open()) {throw std::runtime_error("Failed to open scene file!");}

    std::shared_ptr<LveModel> lveModel = nullptr;
    std::string line, model, material;
    glm::vec3 rotation{}, scale{1.f, 1.f, 1.f}, translation{};
    int count;
    scene >> count;
    getline(scene, line); //clear the line

    for (int i = 0; i < count; i++)
    {
      getline(scene, line);
      model = line;
      getline(scene, line);
      material = line;

      scene >> translation[0] >> translation[1] >> translation[2];
      scene >> scale[0] >> scale[1] >> scale[2];
      scene >> rotation[0] >> rotation[1] >> rotation[2];
      
      lveModel = LveModel::createModelFromFile(lveDevice, model);
      LveGameObject object = LveGameObject::createGameObject();
      object.model = lveModel;
      object.textures = materialHandler->retrieveMaterial(material);
      object.transform.translation = translation;
      object.transform.rotation = rotation;
      object.transform.scale = scale;
      object.write_material(*matLayout, *normalLayout, pool);

      std::cout << object.getId() << '\n';

      gameObjects.emplace(object.getId(), std::move(object));
      
      getline(scene, line); //clear the line
    }
    
    scene.close();
  }

  void LveScene::loadModel(LveDescriptorPool& pool)
  {
    std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "models/cube.obj");
    auto quad = LveGameObject::createGameObject();
    quad.model = lveModel;
    quad.transform.translation = { 0.f, .5f, 0.f };
    quad.transform.scale = { 1.f, 1.f, 1.f };
    quad.textures = materialHandler->retrieveMaterial("materials/wet_rock.thmat");
    quad.write_material(*matLayout, *normalLayout, pool);
    std::cout << quad.getId() << '\n';
    gameObjects.emplace(quad.getId(), std::move(quad));
  }
}
