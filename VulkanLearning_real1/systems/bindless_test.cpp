#include "bindless_test.hpp"
#include "shadow_system.hpp"
#include <vulkan/vulkan_core.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>

namespace lve
{

	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
        uint RID;
	};

	SimpleBindlessSystem::SimpleBindlessSystem(LveDevice& device, VkRenderPass renderPass, 
                                            std::vector<VkDescriptorSetLayout> globalSetLayout) : lveDevice{device}
	{
		createPipeLineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	SimpleBindlessSystem::~SimpleBindlessSystem()
	{
		vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);

	}

	void SimpleBindlessSystem::createPipeLineLayout(std::vector<VkDescriptorSetLayout> &globalSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		//std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(globalSetLayout.size());
		pipelineLayoutInfo.pSetLayouts = globalSetLayout.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;


		if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("\n\nfailed to create pipeline layout\n\n");
		}
	}

	//SHADERS HERE
	void SimpleBindlessSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
		
		//LvePipeline::enableMSAA(pipelineConfig);
	
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		std::vector<std::string> filePaths = { "shaders/compiled/bindless_test.vert.spv",
			"shaders/compiled/bindless_test.frag.spv" };
		lvePipeline = std::make_unique<LvePipeline>(lveDevice, filePaths, pipelineConfig);

	}


	void SimpleBindlessSystem::renderGameObjects(FrameInfo &frameInfo)
	{
		lvePipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
			0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);
        
        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,  pipelineLayout,
			1, 1, &frameInfo.bindlessSet, 0, nullptr);

		for (auto& kv : frameInfo.gameObjects)
		{
			auto& obj = kv.second;
			if (obj.model == nullptr) continue;
			SimplePushConstantData push{};
			push.modelMatrix = obj.transform.mat4();
			push.normalMatrix = obj.transform.normalMatrix();
            push.RID = obj.RID;

			vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(SimplePushConstantData), &push);

			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
	}
}
