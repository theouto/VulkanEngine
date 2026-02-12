#include "normal_spec.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>

namespace lve
{

	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{ 1.f };
        glm::mat4 normalMatrix{1.f};
	};

	NormalSpecPass::NormalSpecPass(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, VkDescriptorSetLayout normalLayout) 
        : lveDevice{device}, layout{normalLayout}
	{
        createPipeLineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	NormalSpecPass::~NormalSpecPass()
	{
		vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);

	}

	void NormalSpecPass::createPipeLineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout, layout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;


		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("\n\nfailed to create pipeline layout\n\n");
		}
	}

	//SHADERS HERE
	void NormalSpecPass::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        LvePipeline::enableAlphaBlending(pipelineConfig);

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		std::vector<std::string> filePaths = { "shaders/compiled/normal_spec.vert.spv",
			"shaders/compiled/normal_spec.frag.spv" };
		lvePipeline = std::make_unique<LvePipeline>(lveDevice, filePaths, pipelineConfig);

	}


	void NormalSpecPass::drawDepth(FrameInfo &frameInfo)
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

            vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                1,
                1,
                &obj.normalSet,
                0,
                nullptr);
			vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(SimplePushConstantData), &push);
			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
	}
}
