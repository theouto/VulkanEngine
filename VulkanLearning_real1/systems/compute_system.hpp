#pragma once

#include "../include/lve_pipeline.hpp"
#include "../include/lve_device.hpp"
#include "../include/lve_frame_info.hpp"

#include <memory>
#include <vector>

namespace lve
{
  class ComputeSystem
  {
    public:

        ComputeSystem(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~ComputeSystem();

		ComputeSystem(const ComputeSystem&) = delete;
		ComputeSystem& operator=(const ComputeSystem&) = delete;

		void compute(FrameInfo &frameInfo, uint32_t width, uint32_t height);
	private:
		void createPipeLineLayout(VkDescriptorSetLayout &globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		LveDevice& lveDevice;
		std::unique_ptr<LvePipeline> lvePipeline;
		VkPipelineLayout pipelineLayout;
  };
};
