#pragma once

#include "lve_device.hpp"
#include "lve_descriptors.hpp"
#include "lve_buffer.hpp"
#include "lve_window.hpp"

namespace lve
{
	class LveTextures
	{
	public:

        enum texType
        {
          COLOR,
          NORMAL,
          SINGLE_UNORM
        };
 
		LveTextures(LveDevice& device, const char *path, texType tType);
		~LveTextures();

		void createTextureImage();
		void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
			VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		VkImageView createImageView(VkImage image, VkFormat format);
		void createTextureImageView();
		void createTextureSampler();
		void generateMipmaps(int32_t texWidth, int32_t texHeight);
        texType getTexType() {return tType;}

		//getter functions, used for destruction
		VkImage& getTextureImage() { return textureImage; }
		VkDeviceMemory& getTextureImageMemory() { return textureImageMemory; }
		VkImageView& getTextureImageView() { return textureImageView; }
		VkSampler& getSampler() { return textureSampler; }
		VkDescriptorImageInfo getDescriptorInfo() 
		{

			VkDescriptorImageInfo descriptorInfo{};

			descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			descriptorInfo.imageView = textureImageView;
			descriptorInfo.sampler = textureSampler;

			return descriptorInfo; 
		}

	private:

		uint32_t mipLevels;
		VkFormat textureFormat;
		texType tType;
        const char *filePath;

		LveDevice& lveDevice;
		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageCreateInfo imageInfo{};
		VkImageView textureImageView;
		VkSampler textureSampler;

    };
}
