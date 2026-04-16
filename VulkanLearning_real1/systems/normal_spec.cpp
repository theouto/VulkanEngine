#include "normal_spec.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>

namespace lve
{

	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{ 1.f };
        glm::mat4 normalMatrix{1.f};
        uint relevantRid[4];
	};

	NormalSpecPass::NormalSpecPass(LveDevice& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> globalSetLayout) 
        : lveDevice{device}, setLayouts{globalSetLayout}
	{
        createPipeLineLayout();
		createPipeline(renderPass);
	}

	NormalSpecPass::~NormalSpecPass()
	{
		vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);

	}

	void NormalSpecPass::createPipeLineLayout()
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
			throw std::runtime_error("\n\nfailed to create pipeline layout\n\n");
		}
	}

	//SHADERS HERE
	void NormalSpecPass::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
        //LvePipeline::enableAlphaBlending(pipelineConfig);
        pipelineConfig.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		std::vector<std::string> filePaths = { "shaders/compiled/normal_spec.vert.spv",
			"shaders/compiled/normal_spec.frag.spv" };
		lvePipeline = std::make_unique<LvePipeline>(lveDevice, filePaths, pipelineConfig);

	}


	void NormalSpecPass::drawDepth(FrameInfo &frameInfo)
	{
		lvePipeline->bind(frameInfo.commandBuffer);

        VkDescriptorSet arr[] = {frameInfo.globalDescriptorSet, frameInfo.bindlessSet};

		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
			0, 2, arr, 0, nullptr);

		for (auto& kv : frameInfo.gameObjects)
		{
			auto& obj = kv.second;
			if (obj.model == nullptr) continue;
			SimplePushConstantData push{};
			push.modelMatrix = obj.transform.mat4();
            push.normalMatrix = obj.transform.normalMatrix();
            push.relevantRid[0] = obj.textures[1];
            push.relevantRid[1] = obj.textures[2];

			vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(SimplePushConstantData), &push);
			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
	}
}
