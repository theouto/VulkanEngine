#include "../include/lve_camera.hpp"
#include "../include/lve_pipeline.hpp"
#include "../include/lve_device.hpp"
#include "../include/lve_model.hpp"
#include "../include/lve_game_object.hpp"
#include "../include/lve_frame_info.hpp"
#include "../include/lve_textures.hpp"

namespace lve
{
  struct Material 
  {
    bool color;
    VkSampler tColor;
    bool spec;
    VkSampler tSpec;
    bool normal;
    VkSampler tNormal;
    bool depth;
    VkSampler tDepth;
  };

  enum class MaterialPass :uint8_t {
    MainColor,
    Transparent,
    Other
  };

  struct MaterialPipeline 
  {
	VkPipeline pipeline;
	VkPipelineLayout layout;
  };

  struct MaterialInstance {
    MaterialPipeline* pipeline;
    VkDescriptorSet materialSet;
    MaterialPass passType;
  };

  struct GLTFMetallic_Roughness {
	MaterialPipeline opaquePipeline;
	MaterialPipeline transparentPipeline;

	VkDescriptorSetLayout materialLayout;

	struct MaterialConstants {
		glm::vec4 colorFactors;
		glm::vec4 metal_rough_factors;
		//padding, we need it anyway for uniform buffers
		glm::vec4 extra[14];
	};

	struct MaterialResources {
        std::unique_ptr<LveTextures> colorImage;
		VkSampler colorSampler;
		std::unique_ptr<LveTextures> metalRoughImage;
		VkSampler metalRoughSampler;
        std::unique_ptr<LveTextures> normalMap;
		VkSampler normalSampler;
		std::unique_ptr<LveTextures> displacementImage;
		VkSampler displacementSampler;
		VkBuffer dataBuffer;
		uint32_t dataBufferOffset;
	};

	LveDescriptorWriter writer;

	void build_pipelines(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
	void clear_resources(VkDevice device);

	MaterialInstance write_material(VkDevice device, MaterialPass pass, const MaterialResources& resources, LveDescriptorPool& descriptorAllocator);
};

}
