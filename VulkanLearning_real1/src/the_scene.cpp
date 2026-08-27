#include "../include/the_scene.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <memory>


namespace lve
{

  LveScene::LveScene(LveDevice &device, LveGameObject::Map& objects, LveRenderer& renderer)
  : lveDevice{device}, gameObjects{objects}, lveRenderer{renderer}
  {
    materialHandler = std::make_unique<LveMaterials>(lveDevice);
  }

  void LveScene::load(std::string file, LveDescriptorPool& pool)
  {
    currentScene = file;
    std::ifstream scene(file.c_str());
    if (!scene.is_open()) {throw std::runtime_error("Failed to open scene file!");}

    scene >> count;
    getline(scene, line); //clear the line

    for (int i = 0; i < count; i++)
    {
      scene >> type;
      getline(scene, line);
      
      if (type == -1) {createObjectHelper(scene, pool);}
      else if (type == 0) createPointLightHelper(scene);
    }

    scene.close();

    for (auto &kv : models)
    {
      kv.second->createInstanceBuffer();
    }

  }

  void LveScene::loadModel(LveGameObject& object, LveDescriptorPool& pool, 
                           LveDescriptorPool& bindlessPool, 
                           LveDescriptorSetLayout& bindlessLayout, 
                           VkDescriptorSet& bindlessSet,
                           const char* path)
  {
    //object.descriptorSet = materialHandler->retrieveMaterial(path, *matLayout, pool);
    changeMaterial(object, bindlessPool, bindlessLayout, bindlessSet, path);
    gameObjects.emplace(object.getId(), std::move(object));
  }

  void LveScene::changeMaterial(LveGameObject& object,
                                LveDescriptorPool& bindlessPool, 
                                LveDescriptorSetLayout& bindlessLayout, 
                                VkDescriptorSet& bindlessSet,
                                const char* path)
  {
    object.matName = path;
    std::vector<uint32_t> arr = materialHandler->retrieveBindless(path, bindlessLayout, bindlessPool, bindlessSet, object);
    for (int i = 0; i < arr.size(); i++) {object.textures[i] = arr[i];}
  }

  void LveScene::saveScene()
  {
    std::ofstream scene(currentScene);
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

    scene.close();
  }

  void LveScene::createObjectHelper(std::ifstream& scene, LveDescriptorPool& pool)
  {
    getline(scene, line);
    name = line;
    getline(scene, line);
    model = line;
    getline(scene, line);
    material = line;

    XXH32_hash_t hash = XXH32(model.c_str(), model.length(), 0);

    scene >> translation[0] >> translation[1] >> translation[2];
    scene >> scale[0] >> scale[1] >> scale[2];
    scene >> rotation[0] >> rotation[1] >> rotation[2];

    uint32_t instanceIndex = retrieveModel(hash, model);

    LveGameObject object = LveGameObject::createGameObject();
    object.model = lveModel;
    object.matName = material;
    object.modelName = model;
    object.instanceHash = hash;
    object.instanceIndex = instanceIndex;

    std::vector<uint32_t> arr = materialHandler->retrieveBindless(material, *lveRenderer.bindlessSetLayout, 
                        *lveRenderer.descriptorPool, lveRenderer.getBindlessLayout(),
                                    object);

    //It's good to have this here as well in the event of a change in model data, necessitating a move away from the
    //previous instance. Sure, I could extradite the data in the model change, but that's a TODO: for a later date.
    for (int i = 0; i < arr.size(); i++) {object.textures[i] = arr[i];}
    object.transform.translation = translation;
    object.transform.rotation = rotation;
    object.transform.scale = scale;
    object.name = name;

    object.model->addInstanceData(scale, translation, rotation, arr, 
                                  materialHandler->modi(XXH32(material.c_str(), material.length(), 0)));

    gameObjects.emplace(object.getId(), std::move(object));

    getline(scene, line); //clear the line
  }

  void LveScene::createObject(std::string name, std::string model,
                              std::string material, glm::vec3 translation,
                              glm::vec3 scale, glm::vec3 rotation,
                              LveDescriptorPool& pool)
  {
    XXH32_hash_t hash = XXH32(model.c_str(), model.length(), 0);
    uint32_t instanceIndex = retrieveModel(hash, model);

    LveGameObject object = LveGameObject::createGameObject();
    object.model = lveModel;
    object.matName = material;
    object.modelName = model;
    object.instanceHash = hash;
    object.instanceIndex = instanceIndex;

    std::vector<uint32_t> arr = materialHandler->retrieveBindless(material, *lveRenderer.bindlessSetLayout, 
                        *lveRenderer.descriptorPool, lveRenderer.getBindlessLayout(),
                                    object);

    //It's good to have this here as well in the event of a change in model data, necessitating a move away from the
    //previous instance. Sure, I could extradite the data in the model change, but that's a TODO: for a later date.
    for (int i = 0; i < arr.size(); i++) {object.textures[i] = arr[i];}
    object.transform.translation = translation;
    object.transform.rotation = rotation;
    object.transform.scale = scale;
    object.name = name;

    object.model->addInstanceData(scale, translation, rotation, arr,
                                  materialHandler->modi(XXH32(material.c_str(), material.length(), 0)));

    gameObjects.emplace(object.getId(), std::move(object));
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

  uint32_t LveScene::retrieveModel(XXH32_hash_t hash, std::string model)
  {
    int instanceCount;
    try {
      lveModel = models.at(hash);
      instanceCount = lveModel->getInstanceCount();} catch (std::out_of_range e)
    {
      lveModel = LveModel::createModelFromFile(lveDevice, model);
      models.emplace(hash, lveModel);
      lveModel = models.at(hash);
      return 0;
    }

    if (instanceCount > 8192)
    {
      std::cout << "new model!\n";
      lveModel = LveModel::createModelFromFile(lveDevice, model);
      models.emplace(hash+1, lveModel);
      lveModel = models.at(hash+1);
      //TODO: fix this inevitable problem that I am sidelining for the moment
    } else {
      lveModel->upInstanceCount();
    }

    return lveModel->getInstanceCount() - 1;
  }
}
