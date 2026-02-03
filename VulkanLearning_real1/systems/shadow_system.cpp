#include "shadow_system.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>

namespace lve
{

  struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

  DirectionalLightSystem::DirectionalLightSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) 
  : lveDevice{device}
  {
    setLayouts.push_back(globalSetLayout);
    createPipeLineLayout();
	createPipeline(renderPass);
  }

  DirectionalLightSystem::~DirectionalLightSystem(){}

  void DirectionalLightSystem::createDescriptorSets()
  {
    auto lightSetLayout = LveDescriptorSetLayout::Builder(lveDevice)        
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
            .build();

    setLayouts.push_back(lightSetLayout->getDescriptorSetLayout());
  }

  void DirectionalLightSystem::createPipeLineLayout()
  {
    VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
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

    pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	std::vector<std::string> filePaths = { "shaders/directional_light.vert.spv",
		"shaders/directional_light.frag.spv" };
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
			push.modelMatrix = obj.transform.mat4();
			push.normalMatrix = obj.transform.normalMatrix();

			vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(SimplePushConstantData), &push);
            vkCmdBindDescriptorSets(
                frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                1,
                1,
                &obj.descriptorSet,
                0,
                nullptr);
			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
  }
}
