#pragma once

#include "../include/lve_camera.hpp"
#include "../include/lve_pipeline.hpp"
#include "../include/lve_device.hpp"
#include "../include/lve_model.hpp"
#include "../include/lve_game_object.hpp"
#include "../include/lve_frame_info.hpp"
#include "./shadow_system.hpp"

#include <memory>
#include <vector>

namespace lve
{
	class SimpleBindlessSystem
	{
        static constexpr int STORAGE_BINDING = 0;
        static constexpr int SAMPLER_BINDING = 1;
        static constexpr int IMAGE_BINDING = 2;

	public:
		SimpleBindlessSystem(LveDevice& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> globalSetLayout);
		~SimpleBindlessSystem();

		SimpleBindlessSystem(const SimpleBindlessSystem&) = delete;
		SimpleBindlessSystem& operator=(const SimpleBindlessSystem&) = delete;

		void renderGameObjects(FrameInfo &frameInfo, glm::mat4 matrix, glm::vec3 lightPos);
	private:
		void createPipeLineLayout(std::vector<VkDescriptorSetLayout> &globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

        glm::mat4 lightSpaceMatrix{1.f};
		LveDevice& lveDevice;
		std::unique_ptr<LvePipeline> lvePipeline;
		VkPipelineLayout pipelineLayout;
	};
}
