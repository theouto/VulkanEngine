#pragma once

#include "lve_device.hpp"
#include "lve_buffer.hpp"
#include "lve_textures.hpp"
#include "../thirdparty/xxHash/xxhash.h"
#include <glm/ext/vector_float3.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>
#include <memory>
#include <vector>

namespace lve
{
	class LveModel
	{

	public:

		struct Vertex
		{
			glm::vec3 position;
			glm::vec3 color;
			glm::vec3 normal{};
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator ==(const Vertex& other) const { return position == other.position && color == other.color 
				&& normal == other.normal && uv == other.uv; }
		};

        struct InstanceData
        {
          alignas(16) glm::vec3 translation;
          alignas(16) glm::vec3 rotation;
          alignas(16) glm::vec3 scale;
          uint32_t RID[8];
          float modifiers[4];
        };

		struct UniformBufferObject {
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		}; 

		struct Builder
		{
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void loadModel(const std::string& filepath);
		};

		LveModel(LveDevice & device, const LveModel::Builder &builder);
		~LveModel();

		LveModel(const LveModel&) = delete;
		LveModel& operator=(const LveModel&) = delete;

      	VkDescriptorPool descriptorPool;

		std::vector<VkDescriptorSet> descriptorSets;
        void upInstanceCount(){instanceCount++;}

		static std::unique_ptr<LveModel> createModelFromFile(LveDevice& device, const std::string &filepath);

        uint32_t addInstanceData(glm::vec3 scale, glm::vec3 translation, glm::vec3 rotation, std::vector<uint32_t> material, std::vector<float> materialModifiers);

        void setScale(uint32_t index, glm::vec3 scale) {instanceData[index].scale = scale;}
        void setTranslation(uint32_t index, glm::vec3 translation) {instanceData[index].translation = translation;}
        void setRotation(uint32_t index, glm::vec3 rotation) {instanceData[index].scale = rotation;}

        glm::vec3 getScale(uint32_t index){return instanceData[index].scale;}
        glm::vec3 getRotation(uint32_t index){return instanceData[index].rotation;}
        glm::vec3 getTranslation(uint32_t index){return instanceData[index].translation;}

        uint32_t getInstanceCount() {return instanceCount;}

		void bind(VkCommandBuffer);
		void draw(VkCommandBuffer);

	private:
		void createVertexBuffers(const std::vector<Vertex> &vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);
		
        std::vector<std::unique_ptr<LveTextures>> textures;
        LveDevice &lveDevice;

		std::unique_ptr<LveBuffer> vertexBuffer;
		uint32_t vertexCount;

        XXH32_hash_t model_name;
        XXH32_hash_t material_name;
        uint32_t instanceCount = 1;

        std::vector<InstanceData> instanceData;
		
		bool hasIndexBuffer = false;
		std::unique_ptr<LveBuffer> indexBuffer;
		uint32_t indexCount;
	};
}
