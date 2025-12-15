#pragma once

#include "../include/lve_textures.hpp"
#include "../include/lve_renderer.hpp"
#include "../include/lve_buffer.hpp"
#include "../include/lve_pipeline.hpp"
#include "../include/lve_descriptors.hpp"
#include "../include/lve_frame_info.hpp"

#include <memory>
#include <vulkan/vulkan_core.h>

#define UNIFORM_BUFFER_SIZE sizeof(glm::mat4)

namespace lve
{

  class SkyboxSystem
  {
    public:
    
    SkyboxSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : 
      lveDevice{device}
    {
      createPipeLineLayout(globalSetLayout);
      createPipeline(renderPass);
    }
    
    ~SkyboxSystem();
    
	void render(FrameInfo &frameInfo);

    private:
		
    void createPipeLineLayout(VkDescriptorSetLayout globalSetLayout);
	void createPipeline(VkRenderPass renderPass);

    std::shared_ptr<LveTextures> cubemapTexture;
	LveDevice& lveDevice;
	std::unique_ptr<LvePipeline> lvePipeline;
	VkPipelineLayout pipelineLayout;
  };

}
