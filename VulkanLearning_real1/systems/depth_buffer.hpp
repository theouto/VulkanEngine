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
	class DepthBuffer
	{
	public:

		DepthBuffer(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~DepthBuffer();

		DepthBuffer(const DepthBuffer&) = delete;
		DepthBuffer& operator=(const DepthBuffer&) = delete;

		void renderGameObjects(FrameInfo &frameInfo);
	private:
		void createPipeLineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

        glm::mat4 lightSpaceMatrix{1.f};
		LveDevice& lveDevice;
		std::unique_ptr<LvePipeline> lvePipeline;
		VkPipelineLayout pipelineLayout;
	};
}
