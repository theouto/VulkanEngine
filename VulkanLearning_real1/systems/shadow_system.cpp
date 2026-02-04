#include "shadow_system.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>

namespace lve
{

  struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{ 1.f };
        glm::mat4 lightSpaceMatrix{1.f};
	};

  DirectionalLightSystem::DirectionalLightSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) 
  : lveDevice{device}
  {
    setLayouts.push_back(globalSetLayout);

    float near_plane = 0.001f, far_plane = 100.0f;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(glm::vec3(-1.0f, 2.0f, -1.f), 
                                  glm::vec3( 0.0f, 0.0f,  0.0f), 
                                  glm::vec3( 0.0f, -1.0f,  0.0f));
    
    lightSpaceMatrix = lightProjection * lightView;

    createPipeLineLayout();
	createPipeline(renderPass);
  }

  DirectionalLightSystem::~DirectionalLightSystem(){}

  void DirectionalLightSystem::createPipeLineLayout()
  {
    VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SimplePushConstantData);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
	pipelineLayoutInfo.pSetLayouts = setLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout");
	}
  }

  void DirectionalLightSystem::createPipeline(VkRenderPass renderPass)
  {
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
    LvePipeline::defaultPipelineShadowInfo(pipelineConfig);

    pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	std::vector<std::string> filePaths = { "shaders/shadowmap.vert.spv",
		"shaders/shadowmap.frag.spv" };
	lvePipeline = std::make_unique<LvePipeline>(lveDevice, filePaths, pipelineConfig);
  }

  void DirectionalLightSystem::drawDepth(FrameInfo &frameInfo)
  {
    lvePipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
			0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

		for (auto& kv : frameInfo.gameObjects)
		{
			auto& obj = kv.second;
			if (obj.model == nullptr) continue;
			SimplePushConstantData push{};
            push.lightSpaceMatrix = lightSpaceMatrix;
			push.modelMatrix = obj.transform.mat4();

			vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
				0, sizeof(SimplePushConstantData), &push);
            
			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
  }
}
