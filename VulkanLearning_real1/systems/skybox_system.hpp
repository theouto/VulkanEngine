#pragma once

#include "../include/lve_textures.hpp"
#include "../include/lve_renderer.hpp"
#include "../include/lve_buffer.hpp"
#include "../include/lve_pipeline.hpp"
#include <memory>

#define UNIFORM_BUFFER_SIZE sizeof(glm::mat4)

namespace lve
{

  class SkyboxSystem
  {
    public:
    
    SkyboxSystem(LveRenderer* lveRender, const char* filename) : lveRenderer{lveRender}  {init(filename);}
    ~SkyboxSystem(){}

    void init(const char* filename);
    void destroy();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, int imageIndex);
    void update(int imageIndex, const glm::mat4& transformation);

    private:

    void createDescriptorSets();

    LveRenderer* lveRenderer = NULL;
    int imageCount = LveSwapChain::MAX_FRAMES_IN_FLIGHT;
    std::shared_ptr<LveTextures> cubemapTexture = NULL;
    std::vector<LveBuffer> buffers;
    std::vector<std::vector<VkDescriptorSet>> descriptorSets;
    VkShaderModule vs = VK_NULL_HANDLE;
    VkShaderModule fs = VK_NULL_HANDLE;

    LvePipeline* lvePipeline = NULL;

  };

}
