#include "skybox_system.hpp"

namespace lve
{
  void SkyboxSystem::~SkyboxSystem()
  {
    for (int i = 0; i < m_uniformBuffers.size(); i++) {
		m_uniformBuffers[i].Destroy(m_pVulkanCore->GetDevice());
	}

	vkDestroyShaderModule(m_pVulkanCore->GetDevice(), m_vs, NULL);
	vkDestroyShaderModule(m_pVulkanCore->GetDevice(), m_fs, NULL);

	delete lvePipeline;
  }

  void SkyboxSystem::init(const char* filename)
  {
    buffers = 
	
	m_cubemapTex.Init(pVulkanCore);
	m_cubemapTex.LoadEctCubemap(pFilename, false);
	
	m_vs = CreateShaderModuleFromText(pVulkanCore->GetDevice(), "../VulkanCore/Shaders/skybox.vert");
	m_fs = CreateShaderModuleFromText(pVulkanCore->GetDevice(), "../VulkanCore/Shaders/skybox.frag");
	
	PipelineConfigInfo pd;
	pd.Device = lveDevice->GetDevice();
	pd.pWindow = pVul->GetWindow();
	pd.vs = m_vs;
	pd.fs = m_fs;
	pd.NumImages = m_numImages;
	pd.ColorFormat = m_pVulkanCore->GetSwapChainFormat();
	pd.DepthFormat = m_pVulkanCore->GetDepthFormat();
	pd.DepthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	pd.IsTexCube = true;
	pd.IsUniform = true;
	
	m_pPipeline = new GraphicsPipelineV2(pd);

	CreateDescriptorSets();
  }


}
