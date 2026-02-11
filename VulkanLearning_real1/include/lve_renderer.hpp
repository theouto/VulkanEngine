#pragma once

#include "lve_textures.hpp"
#include "lve_window.hpp"
#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_model.hpp"
#include "lve_textures.hpp"

#include <cassert>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace lve
{
	class LveRenderer
	{
	public:

		LveRenderer(LveWindow &window, LveDevice& device);
		~LveRenderer();

		LveRenderer(const LveRenderer&) = delete;
		LveRenderer& operator=(const LveRenderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const { return lveSwapChain->getRenderPass(); }
		float getAspectRatio() const { return lveSwapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return isFrameStarted; }
		VkCommandBuffer getCurrentCommandBuffer() const
		{ 
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
				return commandBuffers[currentFrameIndex]; 
		}
		int getFrameIndex() const 
		{ 
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex; 
		}

        VkRenderPass getSwapChainShadowPass() const {return lveSwapChain->getShadowPass();}
        VkDescriptorImageInfo getShadowInfo()
        {
            VkSampler sampler;
            LveTextures::createTextureSampler(lveDevice, sampler);
            VkDescriptorImageInfo descriptorInfo{};

			descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			descriptorInfo.imageView = lveSwapChain->getShadowView();
			descriptorInfo.sampler = sampler;

			return descriptorInfo; 
        }

        VkRenderPass getSwapChainDepthPass() const {return lveSwapChain->getDepthPass();}
        VkDescriptorImageInfo getDepthInfo()
        {
          VkSampler sampler;
          LveTextures::createTextureSampler(lveDevice, sampler);

          VkDescriptorImageInfo descriptorInfo{};

		  descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		  descriptorInfo.imageView = lveSwapChain->getDepthView();
		  descriptorInfo.sampler = sampler;

		  return descriptorInfo;
        }

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void beginShadowRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:

		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

        bool skip = false;
		LveWindow& lveWindow;
		LveDevice& lveDevice;
		std::unique_ptr<LveSwapChain> lveSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

        VkExtent2D extent = {0, 0};
		uint32_t currentImageIndex;
		int currentFrameIndex{0};
		bool isFrameStarted{false};
	};
}
