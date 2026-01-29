#pragma once

#include "../include/lve_renderer.hpp"
#include "../include/lve_buffer.hpp"
#include "../include/lve_pipeline.hpp"
#include "../include/lve_descriptors.hpp"
#include "../include/lve_frame_info.hpp"

#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace lve
{

  class DirectionalLightSystem
  {
    public:

    DirectionalLightSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
    ~DirectionalLightSystem();

    private:

      void createPipeline(VkRenderPass renderPass);
      void createPipeLineLayout(VkDescriptorSetLayout globalSetLayout);

      VkImage drawDepth(FrameInfo &frameInfo);
      VkPipelineLayout pipelineLayout;
      LveDevice& lveDevice;
  };

}
