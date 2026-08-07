#include "bindless_test.hpp"
#include "shadow_system.hpp"
#include <unordered_map>
#include <vulkan/vulkan_core.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <stdexcept>
#include <array>

namespace lve
{

	struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
        uint RIDo;
        uint RID[7];
        float modifiers[4];
        //glm::vec4 position[8192];
        //glm::vec4 rotation[8192];
        //glm::vec4 scale[8192];
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

        VkDescriptorSet sets[] = {frameInfo.globalDescriptorSet, frameInfo.bindlessSet, frameInfo.shadowSet};

        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
			0, 3, sets, 0, nullptr);	

        std::unordered_map<XXH32_hash_t, bool> render;

        for (auto& kv : frameInfo.gameObjects)
		{
		  auto& obj = kv.second;
		  if (obj.model == nullptr) continue;
          try {render.at(obj.instanceHash);
              std::cout << "Not on my watch\n";} catch (std::out_of_range e)
          {
			SimplePushConstantData push{};
			push.modelMatrix = obj.transform.mat4();
			push.normalMatrix = obj.transform.normalMatrix();
            frameInfo.materials.pushValues(push.RID, push.modifiers, obj);

            push.RIDo = obj.RID;

            /*
            for (int i = 0; i < obj.model->getInstanceCount(); i++)
            {
              push.position[i] = obj.model->getTranslations()[i];
              push.scale[i] = obj.model->getScales()[i];
              push.rotation[i] = obj.model->getRotations()[i];
            }
            */

            vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(SimplePushConstantData), &push);

			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);

            render.emplace(obj.instanceHash, true);
          }
		}
	}
}
