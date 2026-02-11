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

  class AOSystem
  {
    public:
    
    AOSystem(LveDevice& device, VkRenderPass renderPass, LveDescriptorPool &pool, VkDescriptorSetLayout image);
    ~AOSystem();
    
	void render(FrameInfo &frameInfo);

    private:
	
    LveDevice& lveDevice;

    void createPipeLineLayout(VkDescriptorSetLayout globalSetLayout);
	void createPipeline(VkRenderPass renderPass);
    void createDescriptorSets(VkDescriptorImageInfo image);

    std::unique_ptr<LveDescriptorPool> globalPool{};
    std::unique_ptr<LveDescriptorSetLayout> AOLayout;
    std::vector<VkDescriptorSetLayout> AODescriptor;
    VkDescriptorSet AODesc;

    std::array<VkSampler, 2> samplers;
	
    std::unique_ptr<LvePipeline> lvePipeline;
	VkPipelineLayout pipelineLayout;
  };

}
