#include "compute_system.hpp"

namespace lve
{
  struct computeConstants
  {
    int frame;
  };


  ComputeSystem::ComputeSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
                              : lveDevice{device}
  {
    createPipeLineLayout(globalSetLayout);
	createPipeline(renderPass);
  }

  ComputeSystem::~ComputeSystem()
  {
    vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
  }

  void ComputeSystem::createPipeLineLayout(VkDescriptorSetLayout &globalSetLayout)
  {
    /*
    VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(computeConstants);
    */
	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	//pipelineLayoutInfo.pushConstantRangeCount = 1;
	//pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("\n\nfailed to create pipeline layout\n\n");
	}
  }

  void ComputeSystem::createPipeline(VkRenderPass renderPass)
  {
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
		
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		lvePipeline = std::make_unique<LvePipeline>(lveDevice, "shaders/compiled/computed.comp.spv", pipelineConfig);
  }

  void ComputeSystem::compute(FrameInfo &frameInfo, uint32_t width, uint32_t height)
  {
    lvePipeline->bindCompute(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout,
			0, 1, &frameInfo.computeSet, 0, nullptr);
 
    /*
    computeConstants push{frameInfo.frameIndex};

    vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
				VK_SHADER_STAGE_COMPUTE_BIT,
				0, sizeof(computeConstants), &push);
    */
    vkCmdDispatch(frameInfo.commandBuffer, std::ceil(width / 16.0), 
                  std::ceil(height / 16.0), 1);
  }
};
