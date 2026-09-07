#include "shadow_system.hpp"

#include <memory>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <iostream>

namespace lve
{

  struct SimplePushConstantData
	{
		glm::mat4 modelMatrix{ 1.f };
        glm::mat4 lightSpaceMatrix{1.f};
        glm::vec3 lightPos{-1.f, 2.f, -2.f};
	};

  DirectionalLightSystem::DirectionalLightSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) 
  : lveDevice{device}
  {
    setLayouts.push_back(globalSetLayout);
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
    pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;

    pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	std::vector<std::string> filePaths = { "shaders/compiled/shadowmap.vert.spv",
		"shaders/compiled/shadowmap.frag.spv" };
	lvePipeline = std::make_unique<LvePipeline>(lveDevice, filePaths, pipelineConfig);
  }

  void DirectionalLightSystem::drawDepth(FrameInfo &frameInfo, glm::mat4 matrix, glm::vec3 lightPos)
  {
    lvePipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
			0, 1, &frameInfo.globalDescriptorSet, 0, nullptr);

        std::unordered_map<XXH32_hash_t, bool> render;

		for (auto& kv : frameInfo.gameObjects)
		{
			auto& obj = kv.second;
			if (obj.model == nullptr) continue;
            try{render.at(obj.instanceHash);} catch (std::out_of_range e)
            {
			SimplePushConstantData push{};
            push.lightSpaceMatrix = matrix;
            push.lightPos = lightPos;

			push.modelMatrix = obj.transform.mat4();

			vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
				0, sizeof(SimplePushConstantData), &push);

			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
            render.emplace(obj.instanceHash, true);
            }
		}
  }

  glm::mat4 DirectionalLightSystem::lightViewProjection(const glm::vec3 &dirLightPos, 
                                                        const glm::vec3 &cameraPosition, float sceneRadius)
  {
      float zNear = 0.01f;
      float zFar = 400.f;
      float lightSize = sceneRadius * 2.f;
      glm::vec3 lightTarget = cameraPosition;
      glm::vec3 lightPosition = lightTarget - dirLightPos * sceneRadius;

      glm::mat4 depthProjectionMatrix = glm::ortho(-lightSize, lightSize, -lightSize, lightSize, zNear, zFar);
      glm::mat4 depthViewMatrix = glm::lookAt(lightPosition, lightTarget, glm::vec3(0.f, 1.f, 0.f));

      return depthProjectionMatrix * depthViewMatrix; 
  }

  //https://github.com/SaschaWillems/Vulkan/blob/master/examples/shadowmappingcascade/shadowmappingcascade.cpp
  //Sascha Willems, you are great
  void DirectionalLightSystem::updateCascades(std::vector<glm::mat4>& matrices, std::vector<float>& cascadeDepth, 
                      float nearClip, float farClip, glm::mat4 invCam, glm::vec3 lightPos)
  {
	float clipRange = farClip - nearClip;

	float minZ = nearClip;
	float maxZ = nearClip + clipRange;

	float range = maxZ - minZ;
	float ratio = maxZ / minZ;

    float cascadeSplits[LveSwapChain::SHADOW_CASCADES];
    float cascadeSplitLambda = 0.95f;

	// Calculate split depths based on view camera frustum
	// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
	for (uint32_t i = 0; i < LveSwapChain::SHADOW_CASCADES; i++) {
		float p = (i + 1) / static_cast<float>(LveSwapChain::SHADOW_CASCADES);
		float log = minZ * std::pow(ratio, p);
		float uniform = minZ + range * p;
		float d = cascadeSplitLambda * (log - uniform) + uniform;
		cascadeSplits[i] = (d - nearClip) / clipRange;
    }

	// Calculate orthographic projection matrix for each cascade
	float lastSplitDist = 0.0;
	for (uint32_t i = 0; i < LveSwapChain::SHADOW_CASCADES; i++) 
    {
	  float splitDist = cascadeSplits[i];

	  glm::vec3 frustumCorners[8] = {
	  	glm::vec3(-1.0f,  1.0f, 0.0f),
	  	glm::vec3( 1.0f,  1.0f, 0.0f),
	  	glm::vec3( 1.0f, -1.0f, 0.0f),
	  	glm::vec3(-1.0f, -1.0f, 0.0f),
	  	glm::vec3(-1.0f,  1.0f, 1.0f),
	  	glm::vec3( 1.0f,  1.0f, 1.0f),
	  	glm::vec3( 1.0f, -1.0f, 1.0f),
	  	glm::vec3(-1.0f, -1.0f, 1.0f),
	  };

	  // Project frustum corners into world space
	  for (uint32_t j = 0; j < 8; j++) {
	  	glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[j], 1.0f);
	  	frustumCorners[j] = invCorner / invCorner.w;
	  }

	  for (uint32_t j = 0; j < 4; j++) {
	  	glm::vec3 dist = frustumCorners[j + 4] - frustumCorners[j];
	  	frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
	  	frustumCorners[j] = frustumCorners[j] + (dist * lastSplitDist);
	  }

	  // Get frustum center
	  glm::vec3 frustumCenter = glm::vec3(0.0f);
	  for (uint32_t j = 0; j < 8; j++) {
	  	frustumCenter += frustumCorners[j];
	  }
	  frustumCenter /= 8.0f;

	  float radius = 0.0f;
	  for (uint32_t j = 0; j < 8; j++) {
	  	float distance = glm::length(frustumCorners[j] - frustumCenter);
	  	radius = glm::max(radius, distance);
	  }
		
      radius = std::ceil(radius * 16.0f) / 16.0f;

      //this is the bit that I need to fix
      radius *= (i+1) * 5.5f;
      //std::cout << "radius " << i << ": " << radius << '\n';

	  glm::vec3 maxExtents = glm::vec3(radius);
	  glm::vec3 minExtents = -maxExtents;

      float zMult = 10.f;

	  glm::vec3 lightDir = normalize(lightPos);
	  glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
	  glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, -zMult, (maxExtents.z - minExtents.z) * zMult);

	  //Store split distance and matrix in cascade
	  //cascades[i].splitDepth = (camera.getNearClip() + splitDist * clipRange) * -1.0f;
	  //cascades[i].viewProjMatrix = lightOrthoMatrix * lightViewMatrix;

      cascadeDepth[i] = nearClip + splitDist * clipRange;
      matrices[i] = lightOrthoMatrix * lightViewMatrix;
	  lastSplitDist = cascadeSplits[i];
    }
  }

}
