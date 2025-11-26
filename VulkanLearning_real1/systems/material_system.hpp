#include "../include/lve_camera.hpp"
#include "../include/lve_pipeline.hpp"
#include "../include/lve_device.hpp"
#include "../include/lve_model.hpp"
#include "../include/lve_game_object.hpp"
#include "../include/lve_frame_info.hpp"
#include "../include/lve_textures.hpp"

namespace lve
{
  /*
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
  */

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

  class MaterialSystem 
  {
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
		std::unique_ptr<LveTextures> specImage;
        std::unique_ptr<LveTextures> normalMap;
		std::unique_ptr<LveTextures> displacementImage;
		LveBuffer dataBuffer;
		uint32_t dataBufferOffset;
	};

	LveDescriptorWriter writer;

	void build_pipelines(LveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout& globalSetLayout);
	void clear_resources(VkDevice device);

	MaterialInstance write_material(LveDevice device, MaterialPass pass, MaterialResources& resources, LveDescriptorPool& descriptorAllocator);
};

}
