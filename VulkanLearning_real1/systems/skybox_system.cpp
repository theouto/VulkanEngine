#include "skybox_system.hpp"

#include <memory>
#include <string>
#include <vector>

namespace lve
{
  SkyboxSystem::~SkyboxSystem()
  {
    /*
    for (int i = 0; i < m_uniformBuffers.size(); i++) {
		m_uniformBuffers[i].Destroy(m_pVulkanCore->GetDevice());
	}
    */

	vkDestroyShaderModule(lveDevice->device(), vs, NULL);
	vkDestroyShaderModule(lveDevice->device(), fs, NULL);

	delete lvePipeline;
  }

  void SkyboxSystem::init(const char* filename)
  {
    //for some reason it will not allow me to initialise the buffer vector with a set size. Will investigate later, probably doing something wrong
    buffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < buffers.size(); i++)
    {
      buffers[i] = std::make_unique<LveBuffer>(
            *lveDevice,
            UNIFORM_BUFFER_SIZE,
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
       buffers[i]->map();
    }

	cubemapTexture = std::make_shared<LveTextures>( *lveDevice, filename, LveTextures::COLOR);	
	
	const PipelineConfigInfo pd = PipelineConfigInfo();	

    std::vector<std::string> skybox = {"../shaders/skybox.vert", "../shaders/skybox.frag"};
	
    LvePipeline dd(*lveDevice, skybox, pd);
    lvePipeline = &dd;

	createDescriptorSets();
  }

  void SkyboxSystem::createDescriptorSets()
  {
    int NumSubmeshes = 1;
	lvePipeline->AllocateDescriptorSets(NumSubmeshes, descriptorSets);

	int NumBindings = 2; // Uniform, Cubemap

	std::vector<VkWriteDescriptorSet> WriteDescriptorSet(imageCount * NumBindings);

	VkDescriptorImageInfo ImageInfo = {
		.sampler = cubemapTexture->getSampler(),
		.imageView = cubemapTexture->getTextureImageView(),
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};

	std::vector<VkDescriptorBufferInfo> BufferInfo_Uniforms(imageCount);

	int WdsIndex = 0;

	for (int ImageIndex = 0; ImageIndex < imageCount; ImageIndex++) {
		BufferInfo_Uniforms[ImageIndex].buffer = buffers[ImageIndex]->getBuffer();
		BufferInfo_Uniforms[ImageIndex].offset = 0;
		BufferInfo_Uniforms[ImageIndex].range = VK_WHOLE_SIZE;
	}

	for (int ImageIndex = 0; ImageIndex < imageCount; ImageIndex++) {
		VkDescriptorSet DstSet = descriptorSets[ImageIndex][0];

		VkWriteDescriptorSet wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = 2,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &BufferInfo_Uniforms[ImageIndex]
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;

		wds = {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = DstSet,
			.dstBinding = 4,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = &ImageInfo
		};

		assert(WdsIndex < WriteDescriptorSet.size());
		WriteDescriptorSet[WdsIndex++] = wds;
	}

	vkUpdateDescriptorSets(lveDevice->device(), 
		                   (uint32_t)WriteDescriptorSet.size(), WriteDescriptorSet.data(), 0, NULL);
  }

}
