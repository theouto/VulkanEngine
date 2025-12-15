#include "skybox_system.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace lve
{
  struct SkyboxPushConstants
  {
	glm::mat4 view{ 1.f };
  };

  SkyboxSystem::~SkyboxSystem(){}

  void SkyboxSystem::createPipeLineLayout(VkDescriptorSetLayout globalSetLayout)
  {
    VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SkyboxPushConstants);

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout");
	}
  }

  void SkyboxSystem::createPipeline(VkRenderPass renderPass)
  {
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.attributeDescriptions.clear();
		pipelineConfig.bindingDescriptions.clear();
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		std::vector<std::string> filePaths = { "shaders/skybox.vert.spv",
			"shaders/skybox.frag.spv" };
		lvePipeline = std::make_unique<LvePipeline>(lveDevice, 
			filePaths, pipelineConfig);
  }

  void SkyboxSystem::render(FrameInfo& frameInfo)
  {
    lvePipeline->bind(frameInfo.commandBuffer);
    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
			0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

    SkyboxPushConstants uboi{};
    uboi.view = frameInfo.camera.getView();

    vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(SkyboxPushConstants), &uboi);

    vkCmdDraw(frameInfo.commandBuffer, 36, 1, 0, 0);
  }

}
