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

    scene >> count;
    getline(scene, line); //clear the line

    for (int i = 0; i < count; i++)
    {
      scene >> type;
      getline(scene, line);
      if (type == -1) {createObjectHelper(scene, pool); std::cout << "chosen!\n";}
      else if (type == 0) createPointLightHelper(scene);
    }
    
    scene.close();
  }

  void LveScene::loadModel(LveGameObject& object, LveDescriptorPool& pool, const char* path)
  {
    object.textures = materialHandler->retrieveMaterial(path);
    object.write_material(*matLayout, *normalLayout, pool);
    gameObjects.emplace(object.getId(), std::move(object));
  }

  void LveScene::saveScene()
  {
    std::ofstream scene("./scenes/test_scene.ths");
    if (!scene.is_open()) {throw std::runtime_error("Failed to open scene file!");}

    scene << gameObjects.size() << '\n';
    for (auto &kv : gameObjects)
    {
      scene << kv.second.type << '\n';

      scene << kv.second.name << '\n';
      if (kv.second.type == -1)
      {
        scene << kv.second.modelName << '\n'
              << kv.second.matName << '\n';

        scene << kv.second.transform.translation[0] << " "
              << kv.second.transform.translation[1] << " "
              << kv.second.transform.translation[2] << '\n';

        scene << kv.second.transform.scale[0] << " "
              << kv.second.transform.scale[1] << " "
              << kv.second.transform.scale[2] << '\n';

        scene << kv.second.transform.rotation[0] << " "
              << kv.second.transform.rotation[1] << " "
              << kv.second.transform.rotation[2] << '\n';
      } else if (kv.second.type == 0)
      {
        //TODO: Update docs to account for this shit that I just made up on the spot
        scene << kv.second.color[0] << " "
              << kv.second.color[1] << " "
              << kv.second.color[2] << '\n';

        scene << kv.second.transform.translation[0] << " "
              << kv.second.transform.translation[1] << " "
              << kv.second.transform.translation[2] << '\n';

        scene << kv.second.transform.scale.x << " "
              << kv.second.pointLight->lightIntensity << '\n';
      }
    }
  }

  void LveScene::createObjectHelper(std::ifstream& scene, LveDescriptorPool& pool)
  {
    getline(scene, line);
    name = line;
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
    object.matName = material;
    object.modelName = model;
    object.textures = materialHandler->retrieveMaterial(material);
    object.transform.translation = translation;
    object.transform.rotation = rotation;
    object.transform.scale = scale;
    object.name = name;
    object.write_material(*matLayout, *normalLayout, pool);
    gameObjects.emplace(object.getId(), std::move(object));

    getline(scene, line); //clear the line

  }

  void LveScene::createPointLightHelper(std::ifstream& scene)
  {
    getline(scene, name);
    scene >> color[0] >> color[1] >> color[2];
    scene >> translation[0] >> translation[1] >> translation[2];
    scene >> radius >> intensity;
    getline(scene, line);

    LveGameObject light = LveGameObject::makePointLight(intensity, radius, color);
    light.name = name;
    light.transform.translation = translation;
    gameObjects.emplace(light.getId(), std::move(light));
  }
}
