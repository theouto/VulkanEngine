#pragma once

#include "lve_game_object.hpp"
#include "lve_textures.hpp"
#include "lve_device.hpp"

#include <unordered_map>
#include <vector>
#include <map>
#include <string>

namespace lve
{
  class LveMaterials
  {
    public:

    LveMaterials(LveDevice &device);
    ~LveMaterials(){}

    VkDescriptorSet retrieveMaterial(const std::string path,
                                     LveDescriptorSetLayout& descLayout,
                                     LveDescriptorPool& descPool);

    std::vector<uint32_t> retrieveBindless(const std::string path,
                                           LveDescriptorSetLayout& descLayout,
                                           LveDescriptorPool& descPool,
                                           VkDescriptorSet& bindlessSet,
                                           LveGameObject& object);

    static VkDescriptorSet writeMaterial(std::vector<std::shared_ptr<LveTextures>> textures,
                                          LveDescriptorSetLayout& descLayout, 
                                          LveDescriptorPool& descPool);

    void writeBindless(std::vector<std::shared_ptr<LveTextures>> textures,
                               LveDescriptorSetLayout& descLayout, 
                               LveDescriptorPool& descPool,
                               VkDescriptorSet& bindlessSet);

    void pushValues(uint* RID, float* modified, LveGameObject& object);
    std::vector<uint32_t>& keys() {return _keys;}
    std::vector<float>& modi(uint32_t hash) {return modifiers.at(hash);}
    std::vector<float>& patchwork() {return modifiers.at(_keys[0]);} //bear with me here, this will be fixed, but for now I want a system working
                                                                        //I know it's bad, but unlike the thing below I wlil eventually fix it

    //this will be removed once the bindless descriptors stop being an experiment, but for nowwwww yeahhhhhhhhh
    //Update: I lied lol this stays here
    static std::vector<std::shared_ptr<LveTextures>> write_test(LveDevice& lveDevice);

    private:

    uint32_t currArr = 2;

    std::vector<uint32_t> _keys;
    std::vector<std::shared_ptr<LveTextures>> totalTextures;

    LveDevice &lveDevice;
    std::unordered_map<unsigned int,
                       std::pair<VkDescriptorSet,
                       std::vector<std::shared_ptr<LveTextures>>>> loadedMaterials;

    std::unordered_map<unsigned int,
                       std::vector<uint32_t>> bindlessTextureSet;

    //The point here is to trade memory for less writes
    std::unordered_map<unsigned int,
                       uint32_t> textures;

    std::unordered_map<uint32_t, std::vector<float>> modifiers;
  };
}
