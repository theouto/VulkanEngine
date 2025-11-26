#include "./material_system.hpp"

namespace lve
{
 /* 
  MaterialInstance MaterialSystem::write_material(LveDevice device, LveDescriptorSetLayout descLayout, 
                                                  std::vector<std::unique_ptr<LveTextures>>& resources, LveDescriptorPool& descriptorAllocator, LveBuffer lveBuffer)
  {
    MaterialInstance matData;
	matData.passType = pass;
	if (pass == MaterialPass::Transparent) {
		matData.pipeline = &transparentPipeline;
	}
	else {
		matData.pipeline = &opaquePipeline;
	}

	descriptorAllocator.allocateDescriptor(materialLayout, matData.materialSet);

    auto colorInfo = resources.colorImage->getDescriptorInfo();
    auto specInfo = resources.specImage->getDescriptorInfo();
    auto normInfo = resources.normalMap->getDescriptorInfo();
    auto dispInfo = resources.displacementImage->getDescriptorInfo();
    auto buffInfo = resources.dataBuffer.descriptorInfo();

  //writer.clear();
	writer.writeBuffer(0, &buffInfo);
	writer.writeImage(1, &colorInfo);
	writer.writeImage(2, &specInfo);
    writer.writeImage(3, &normInfo);
	writer.writeImage(4, &dispInfo);


	writer.build(matData.materialSet);

	return matData;
  }
*/

}
