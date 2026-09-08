#include "../include/lve_game_object.hpp"
#include "../include/lve_swap_chain.hpp"

#include <vector>
#include <format>
#include <iostream>

namespace lve
{
	glm::mat4 TransformComponent::mat4() 
	{
		const float c3 = glm::cos(0.f);
		const float s3 = glm::sin(0.f);
		const float c2 = glm::cos(0.f);
		const float s2 = glm::sin(0.f);
		const float c1 = glm::cos(0.f);
		const float s1 = glm::sin(0.f);
		return glm::mat4
		{
			{
				(c1 * c3 + s1 * s2 * s3),
				(c2 * s3),
				(c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				(c3 * s1 * s2 - c1 * s3),
				(c2 * c3),
				(c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				(c2 * s1),
				(-s2),
				(c1 * c2),
				0.0f,
			},
			//{translation.x, translation.y, translation.z, 1.0f} 
            glm::vec4{glm::vec3{0.f}, 1.f}};
	}

	glm::mat3 TransformComponent::normalMatrix()
	{
		const float c3 = glm::cos(0.f);
		const float s3 = glm::sin(0.f);
		const float c2 = glm::cos(0.f);
		const float s2 = glm::sin(0.f);
		const float c1 = glm::cos(0.f);
		const float s1 = glm::sin(0.f);
		const glm::vec3 invScale{1.f};

		return glm::mat3{
			{
				invScale.x * (c1 * c3 + s1 * s2 * s3),
				invScale.x * (c2 * s3),
				invScale.x * (c1 * s2 * s3 - c3 * s1),
			},
			{
				invScale.y * (c3 * s1 * s2 - c1 * s3),
				invScale.y * (c2 * c3),
				invScale.y * (c1 * c3 * s2 + s1 * s3),
			},
			{
				invScale.z * (c2 * s1),
				invScale.z * (-s2),
				invScale.z * (c1 * c2),
			},
		};
	}
 
	LveGameObject LveGameObject::makePointLight(float intensity, float radius, glm::vec3 color)
	{
		LveGameObject gameObj = LveGameObject::createGameObject();
		gameObj.color = color;
        gameObj.name = std::format("Point Light {}", gameObj.getId());
		gameObj.transform.scale.x = radius;
		gameObj.pointLight = std::make_unique<PointLightComponent>();
		gameObj.pointLight->lightIntensity = intensity;
        gameObj.type = 0;
		return gameObj;
	}

    void LveGameObject::update()
    {
      if (type == -1)
      {
        model->setRotation(instanceIndex, transform.rotation);
        model->setScale(instanceIndex, transform.scale);
        model->setTranslation(instanceIndex, transform.translation);
        model->setMaterial(instanceIndex, textures, modifiers);
      }
    }
}
