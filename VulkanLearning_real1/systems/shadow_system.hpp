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
 
    static constexpr uint32_t WIDTH = 4096;
    static constexpr uint32_t HEIGHT = 4096;


    DirectionalLightSystem(LveDevice& device,VkRenderPass renderPass ,VkDescriptorSetLayout globalSetLayout);
    ~DirectionalLightSystem();

    static glm::mat4 lightViewProjection(const glm::vec3 &dirLightPos, 
                    const glm::vec3 &cameraPosition, float sceneRadius); 

    void drawDepth(FrameInfo &frameInfo, glm::mat4 matrix, glm::vec3 lightPos);
    static std::vector<glm::mat4> getLightSpaceMatrices(std::vector<float> depthValues,
                                                 const glm::mat4& projView, const glm::vec3 rot,
                                                 const float nearPlane, const float farPlane);

    private:

      void createPipeline(VkRenderPass renderPass);
      void createPipeLineLayout();
      static std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4 &projView);
      static glm::mat4 getLightSpaceMatrix(const glm::mat4& projView, const glm::vec3 rot,
                                    const float nearPlane, const float farPlane);

      glm::mat4 lightSpaceMatrix{1.f};
      std::vector<VkDescriptorSetLayout> setLayouts = {};
      VkPipelineLayout pipelineLayout;
      LveDevice& lveDevice;
      std::unique_ptr<LvePipeline> lvePipeline;

  };

}
