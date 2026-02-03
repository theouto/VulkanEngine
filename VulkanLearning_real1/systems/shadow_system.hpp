#pragma once

#include "../include/lve_renderer.hpp"
#include "../include/lve_buffer.hpp"
#include "../include/lve_pipeline.hpp"
#include "../include/lve_descriptors.hpp"
#include "../include/lve_frame_info.hpp"

#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <vector>

namespace lve
{

  class DirectionalLightSystem
  {
    public:
 
    static constexpr uint32_t WIDTH = 2048;
    static constexpr uint32_t HEIGHT = 2048;


    DirectionalLightSystem(LveDevice& device,VkRenderPass renderPass ,VkDescriptorSetLayout globalSetLayout);
    ~DirectionalLightSystem();

    private:

      void createPipeline(VkRenderPass renderPass);
      void createPipeLineLayout();
      void createRenderer();
      void createDescriptorSets();

      std::vector<VkDescriptorSetLayout> setLayouts = {};
      void drawDepth(FrameInfo &frameInfo);
      VkPipelineLayout pipelineLayout;
      LveDevice& lveDevice;
      std::unique_ptr<LvePipeline> lvePipeline;

  };

}
