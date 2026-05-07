#include "shadow_system.hpp"
#include <memory>
#include <vulkan/vulkan_core.h>

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

		for (auto& kv : frameInfo.gameObjects)
		{
			auto& obj = kv.second;
			if (obj.model == nullptr) continue;
			SimplePushConstantData push{};
            push.lightSpaceMatrix = matrix;
            push.lightPos = lightPos;

			push.modelMatrix = obj.transform.mat4();

			vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
				0, sizeof(SimplePushConstantData), &push);
            
			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
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

  //https://learnopengl.com/Guest-Articles/2021/CSM
  std::vector<glm::vec4> DirectionalLightSystem::getFrustumCornersWorldSpace(const glm::mat4& projView)
  {
    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x)
    {
        for (unsigned int y = 0; y < 2; ++y)
        {
            for (unsigned int z = 0; z < 2; ++z)
            {
                const glm::vec4 pt = 
                    projView * glm::vec4(
                        2.0f * x - 1.0f,
                        2.0f * y - 1.0f,
                        2.0f * z - 1.0f,
                        1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }
    
    return frustumCorners;
  }

  glm::mat4 DirectionalLightSystem::getLightSpaceMatrix(const glm::mat4& projView, const glm::vec3 rot,
                                                        const float nearPlane, const float farPlane)
  {
    const auto corners = getFrustumCornersWorldSpace(projView);

    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : corners)
    {
        center += glm::vec3(v);
    }
    center /= corners.size();

    const auto lightView = glm::lookAt(center + rot, center, glm::vec3(0.0f, 1.0f, 0.0f));

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();
    for (const auto& v : corners)
    {
        const auto trf = lightView * v;
        minX = std::min(minX, trf.x);
        maxX = std::max(maxX, trf.x);
        minY = std::min(minY, trf.y);
        maxY = std::max(maxY, trf.y);
        minZ = std::min(minZ, trf.z);
        maxZ = std::max(maxZ, trf.z);
    }

    // Tune this parameter according to the scene
    constexpr float zMult = 10.0f;
    if (minZ < 0)
    {
        minZ *= zMult;
    }
    else
    {
        minZ /= zMult;
    }
    if (maxZ < 0)
    {
        maxZ /= zMult;
    }
    else
    {
        maxZ *= zMult;
    }

    const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
    return lightProjection * lightView;
  }

  std::vector<glm::mat4> DirectionalLightSystem::getLightSpaceMatrices(std::vector<float> depthValues,
                                                 const glm::mat4& projView, const glm::vec3 rot,
                                                 const float nearPlane, const float farPlane)
  {
    std::vector<glm::mat4> ret;
    for (size_t i = 0; i < depthValues.size() + 1; ++i)
    {
        if (i == 0)
        {
            ret.push_back(getLightSpaceMatrix(projView, rot, nearPlane, depthValues[i]));
        }
        else if (i < depthValues.size())
        {
            ret.push_back(getLightSpaceMatrix(projView, rot, depthValues[i - 1], depthValues[i]));
        }
        else
        {
            ret.push_back(getLightSpaceMatrix(projView, rot, depthValues[i - 1], farPlane));
        }
    }
    return ret;
  }
}
