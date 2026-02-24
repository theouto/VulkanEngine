#include "imgui_setup.hpp"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan_core.h>

namespace lve 
{
  Imgui_LVE::Imgui_LVE(LveDevice &device, LveRenderer &render, LveWindow &window) 
      : lveDevice{device}, lveWindow{window}, lveRenderer{render}
  {
    init();
  }

  void Imgui_LVE::init()
  {
    // Initialize ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    	VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;

	VkDescriptorPool imguiPool;
	vkCreateDescriptorPool(lveDevice.device(), &pool_info, nullptr, &imguiPool);
    pool = imguiPool;

    ImGui_ImplGlfw_InitForVulkan(lveWindow.getGLFWwindow(), true);

    // this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = lveDevice.getInstance();
	init_info.PhysicalDevice = lveDevice.getPhysicalDevice();
	init_info.Device = lveDevice.device();
	init_info.Queue = lveDevice.graphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = pool;
	init_info.MinImageCount = LveSwapChain::MAX_FRAMES_IN_FLIGHT;
	init_info.ImageCount = LveSwapChain::MAX_FRAMES_IN_FLIGHT;
	init_info.UseDynamicRendering = false;
    init_info.QueueFamily = lveDevice.findPhysicalQueueFamilies().graphicsFamily;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = nullptr;

    ImGui_ImplVulkan_Init(&init_info);
  }

  void Imgui_LVE::draw(VkCommandBuffer commandBuffer, int index)
  {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame(); 
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();
    ImGui::Render();

    VkRenderingAttachmentInfo colorAttachment = 
                        attachment_info(lveRenderer.getSwapChainImageView(index), 
                        nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderingInfo renderInfo = rendering_info(lveWindow.getExtent(), &colorAttachment, nullptr);

	vkCmdBeginRendering(commandBuffer, &renderInfo);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

	vkCmdEndRendering(commandBuffer);

    ImGui::EndFrame();
  }

  VkRenderingAttachmentInfo Imgui_LVE::attachment_info(VkImageView imageView, 
                             VkClearValue* clear, VkImageLayout layout)
  {
    VkRenderingAttachmentInfo colorAttachment {};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.pNext = nullptr;

    colorAttachment.imageView = imageView;
    colorAttachment.imageLayout = layout;
    colorAttachment.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    if (clear) {
        colorAttachment.clearValue = *clear;
    }

    return colorAttachment;
  }

  VkRenderingInfo Imgui_LVE::rendering_info(VkExtent2D extent, VkRenderingAttachmentInfo* attachment, 
                                            VkRenderingAttachmentInfo* depth)
  {
    VkRenderingInfo colorAttachment {};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.pNext = nullptr;

    colorAttachment.renderArea.extent = extent;
    colorAttachment.pColorAttachments = attachment;
    colorAttachment.pDepthAttachment = depth;

    return colorAttachment;
  }
}
