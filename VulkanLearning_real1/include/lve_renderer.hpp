#pragma once

#include "lve_descriptors.hpp"
#include "lve_game_object.hpp"
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

        VkImageView getSwapChainImageView(int index) {return lveSwapChain->getImageView(index);}

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

        VkRenderPass getSwapChainNormalPass() const {return lveSwapChain->getNormalPass();}
        VkDescriptorImageInfo getNormalInfo()
        {
          VkSampler sampler;
          LveTextures::createTextureSampler(lveDevice, sampler);

          VkDescriptorImageInfo descriptorInfo{};

		  descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		  descriptorInfo.imageView = lveSwapChain->getNormalView();
		  descriptorInfo.sampler = sampler;

		  return descriptorInfo;
        }

        void loadUboInfo(std::vector<std::shared_ptr<LveBuffer>> ubos)
        {
            uboInfo.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
            for (int i = 0; i < LveSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
            {
                uboInfo[i] = ubos[i]->descriptorInfo();
            }
        }

        VkDescriptorBufferInfo getUboInfo(uint32_t index) {return uboInfo[index];}
        VkExtent2D getSwapChainExtent() {return lveSwapChain->getSwapChainExtent();}

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
        void beginNormalRenderPass(VkCommandBuffer commandBuffer);
        void beginDepthRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);


         private:
		LveDevice& lveDevice;
        //what the fuck am I doing
        public: 

        std::unique_ptr<LveDescriptorPool> globalPool = LveDescriptorPool::Builder(lveDevice)
            .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT * LveGameObject::MAX_OBJECTS * 4)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT * LveGameObject::MAX_OBJECTS * 2)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LveSwapChain::MAX_FRAMES_IN_FLIGHT * LveGameObject::MAX_OBJECTS * 4)
            .build();

        //Risky until I find a better way
        //Why do I get the feeling that I'll be staring at this comment 6 months from now?
        std::unique_ptr<LveDescriptorSetLayout> globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)            
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
            .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
            .build();

        VkDescriptorSetLayout getGlobalLayout() {return globalSetLayout->getDescriptorSetLayout();}
        VkDescriptorSet getLayout(uint32_t index) {return globalSetLayouts[index];}
        
        void generateDescriptors()
        {
            globalSetLayouts.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
            
            for(int i = 0; i < LveSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
            {
                auto bufferInfo = getUboInfo(i);
                auto shadowInfo = getShadowInfo();
                auto depthInfo = getDepthInfo();
                auto normalSpecInfo = getNormalInfo();

                LveDescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &bufferInfo) 
                    .writeImage(1, &shadowInfo)
                    .writeImage(2, &depthInfo)
                    .writeImage(3, &normalSpecInfo)
                    .build(globalSetLayouts[i]);
            }
        }

      void updateDescriptors()
      {
            for(int i = 0; i < LveSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
            {
                auto bufferInfo = getUboInfo(i);
                auto shadowInfo = getShadowInfo();
                auto depthInfo = getDepthInfo();
                auto normalSpecInfo = getNormalInfo();

                LveDescriptorWriter(*globalSetLayout, *globalPool)
                    .writeBuffer(0, &bufferInfo) 
                    .writeImage(1, &shadowInfo)
                    .writeImage(2, &depthInfo)
                    .writeImage(3, &normalSpecInfo)
                    .overwrite(globalSetLayouts[i]);
            }

      }

	private:

		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

        bool skip = false;
		LveWindow& lveWindow;

        std::vector<VkDescriptorBufferInfo> uboInfo;

        std::vector<VkDescriptorSet> globalSetLayouts;

        std::unique_ptr<LveSwapChain> lveSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

        VkExtent2D extent = {0, 0};
		uint32_t currentImageIndex;
		int currentFrameIndex{0};
		bool isFrameStarted{false};
	};
}
