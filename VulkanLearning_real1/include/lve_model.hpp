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
			//glm::vec3 color;
			glm::vec3 normal{};
			glm::vec2 uv{};

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator ==(const Vertex& other) const { return position == other.position //&& color == other.color 
				&& normal == other.normal && uv == other.uv; }
		};

        struct InstanceData
        {
          glm::vec4 modelMatrixI;
          glm::vec4 modelMatrixII;
          glm::vec4 modelMatrixIII;
          glm::vec4 modelMatrixIV;

          alignas(16) glm::vec3 normalMatrixI;
          alignas(16) glm::vec3 normalMatrixII;
          alignas(16) glm::vec3 normalMatrixIII;
          
          alignas(16) glm::ivec3 RIDone;
          alignas(16) glm::ivec3 RIDtwo;
          glm::vec4 modifiers;

          static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
          static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
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

		static std::unique_ptr<LveModel> createModelFromFile(LveDevice& device, const std::string &filepath);

        uint32_t addInstanceData(glm::mat4 modelMatrix, glm::mat3 normalMatrix, std::vector<uint32_t> material, std::vector<float> materialModifiers);

        void createInstanceBuffer();
        void updateBuffer();

        void setModelMatrix(uint32_t index, glm::mat4 modelMatrix) 
        {
          instanceData[index].modelMatrixI = modelMatrix[0];
          instanceData[index].modelMatrixII = modelMatrix[1];
          instanceData[index].modelMatrixIII = modelMatrix[2];
          instanceData[index].modelMatrixIV = modelMatrix[3];
        }

        void setNormalMatrix(uint32_t index, glm::mat3 normalMatrix) 
        {
          instanceData[index].normalMatrixI = normalMatrix[0];
          instanceData[index].normalMatrixII = normalMatrix[1];
          instanceData[index].normalMatrixIII = normalMatrix[2];
        }

        void setMaterial(uint32_t index, uint32_t* RID, float* modi)
        {
          instanceData[index].RIDone = glm::vec3{RID[0], RID[1], RID[2]};
          instanceData[index].RIDtwo = glm::vec3{RID[3], RID[4], RID[5]};
          instanceData[index].modifiers = glm::vec4{modi[0], modi[1], modi[2], modi[3]};
        }

        void flush() {instanceBuffer->flush();}

        /* I don't use these for anything...
        glm::mat4 getModelMatrix(uint32_t index){return glm::mat4{instanceData[index].modelMatrixI,
                                                                  instance};}
        glm::mat3 getNormalMatrix(uint32_t index){return instanceData[index].normalMatrix;}
        */

        uint32_t getInstanceCount() {return instanceData.size();}

		void bind(VkCommandBuffer);
		void draw(VkCommandBuffer);

        void updateInstances();
        void updateMaterial();

	private:
		void createVertexBuffers(const std::vector<Vertex> &vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);
		
        std::vector<std::unique_ptr<LveTextures>> textures;
        LveDevice &lveDevice;

        bool createdbuffer = false;
		std::unique_ptr<LveBuffer> vertexBuffer;
		uint32_t vertexCount;

        XXH32_hash_t model_name;
        XXH32_hash_t material_name;

        std::vector<InstanceData> instanceData;
        std::unique_ptr<LveBuffer> instanceBuffer = nullptr;

		bool hasIndexBuffer = false;
		std::unique_ptr<LveBuffer> indexBuffer;
		uint32_t indexCount;
	};
}
