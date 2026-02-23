#include "../include/lve_game_object.hpp"
#include "../include/lve_swap_chain.hpp"

#include <vector>
#include <iostream>
#include <vulkan/vulkan_core.h>

namespace lve
{
	glm::mat4 TransformComponent::mat4() 
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		return glm::mat4
		{
			{
				scale.x * (c1 * c3 + s1 * s2 * s3),
				scale.x * (c2 * s3),
				scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				scale.y * (c3 * s1 * s2 - c1 * s3),
				scale.y * (c2 * c3),
				scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				scale.z * (c2 * s1),
				scale.z * (-s2),
				scale.z * (c1 * c2),
				0.0f,
			},
			{translation.x, translation.y, translation.z, 1.0f} };
	}

    void LveGameObject::createDescriptorSets()
    {
      for (size_t i = 0; i < LveSwapChain::MAX_FRAMES_IN_FLIGHT; i++) 
      {
        //something
      }

    }

	glm::mat3 TransformComponent::normalMatrix()
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 invScale = 1.0f / scale;

		return glm::mat3{
			{
				invScale.x * (c1 * c3 + s1 * s2 * s3),
				invScale.x * (c2 * s3),
				invScale.x * (c1 * s2 * s3 - c3 * s1),
			},
			{
				invScale.y* (c3 * s1 * s2 - c1 * s3),
				invScale.y* (c2 * c3),
				invScale.y * (c1 * c3 * s2 + s1 * s3),
			},
			{
				invScale.z* (c2 * s1),
				invScale.z* (-s2),
				invScale.z * (c1 * c2),
			},
		};
	}
 
	LveGameObject LveGameObject::makePointLight(float intensity, float radius, glm::vec3 color)
	{
		LveGameObject gameObj = LveGameObject::createGameObject();
		gameObj.color = color;
		gameObj.transform.scale.x = radius;
		gameObj.pointLight = std::make_unique<PointLightComponent>();
		gameObj.pointLight->lightIntensity = intensity;
		return gameObj;
	}

    std::vector<VkDescriptorSet> LveGameObject::write_material(LveDescriptorSetLayout& descLayout,
                                       std::vector<std::shared_ptr<LveTextures>> textures,
                                       LveDescriptorSetLayout& normalLayout,
                                       LveDescriptorPool& descPool)
    {
      VkDescriptorSet descriptorSet{}, normalSet{};

      auto colorInfo = textures[0]->getDescriptorInfo();
      auto specInfo = textures[1]->getDescriptorInfo();
      auto normInfo = textures[2]->getDescriptorInfo();
      auto dispInfo = textures[3]->getDescriptorInfo();
      auto AOInfo = textures[4]->getDescriptorInfo();
      auto metalInfo = textures[5]->getDescriptorInfo();

      LveDescriptorWriter(descLayout, descPool)
                .writeImage(1, &colorInfo) // colour
                .writeImage(2, &specInfo) //spec 
                .writeImage(3, &normInfo) //normal
                .writeImage(4, &dispInfo) //displacement
                .writeImage(5, &AOInfo)
                .writeImage(6, &metalInfo)
                .build(descriptorSet);

      LveDescriptorWriter(normalLayout, descPool)
                .writeImage(0, &normInfo)
                .writeImage(1, &specInfo)
                .build(normalSet);

      std::vector<VkDescriptorSet> ret = {descriptorSet, normalSet};
      return ret;
    }
}
