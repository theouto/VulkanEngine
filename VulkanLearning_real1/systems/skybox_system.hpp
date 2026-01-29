#pragma once

#include "../include/lve_textures.hpp"
#include "../include/lve_renderer.hpp"
#include "../include/lve_buffer.hpp"
#include "../include/lve_pipeline.hpp"
#include "../include/lve_descriptors.hpp"
#include "../include/lve_frame_info.hpp"
#include "../include/lve_descriptors.hpp"

#include <memory>
#include <vulkan/vulkan_core.h>

#define UNIFORM_BUFFER_SIZE sizeof(glm::mat4)

namespace lve
{

  class SkyboxSystem
  {
    public:
    
    SkyboxSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout,
                 LveDescriptorPool &pool);
    ~SkyboxSystem();
    
	void render(FrameInfo &frameInfo);

    private:
	
    LveDevice& lveDevice;
    std::shared_ptr<LveTextures> fakebox = std::make_shared<LveTextures>(lveDevice,
                                 "textures/MorningSkyHDRI011A_1K_TONEMAPPED.jpg", LveTextures::COLOR);

    void createPipeLineLayout(std::vector<VkDescriptorSetLayout> globalSetLayout);
	void createPipeline(VkRenderPass renderPass);
    void createDescriptorSets(VkDescriptorSetLayout globalSetLayout);

    std::unique_ptr<LveDescriptorPool> globalPool{};
    std::unique_ptr<LveDescriptorSetLayout> skyLayout;
    std::vector<VkDescriptorSetLayout> skyDescriptors;
    VkDescriptorSet skyDesc;
    std::shared_ptr<LveTextures> cubemapTexture;
	
    std::unique_ptr<LvePipeline> lvePipeline;
	VkPipelineLayout pipelineLayout;
  };

}
