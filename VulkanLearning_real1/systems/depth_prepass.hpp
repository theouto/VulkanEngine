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

  class DepthPrePass
  {
    public:
 
    static constexpr uint32_t WIDTH = 4096;
    static constexpr uint32_t HEIGHT = 4096;


    DepthPrePass(LveDevice& device,VkRenderPass renderPass ,VkDescriptorSetLayout globalSetLayout);
    ~DepthPrePass();

    void drawDepth(FrameInfo &frameInfo);

    private:

      void createPipeline(VkRenderPass renderPass);
      void createPipeLineLayout(VkDescriptorSetLayout globalSetLayout);

      glm::mat4 lightSpaceMatrix{1.f};
      std::vector<VkDescriptorSetLayout> setLayouts = {};
      VkPipelineLayout pipelineLayout;
      LveDevice& lveDevice;
      std::unique_ptr<LvePipeline> lvePipeline;

  };

}
