#pragma once

#include "imconfig.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>

#include "../include/lve_device.hpp"
#include "../include/lve_window.hpp"
#include "../include/lve_renderer.hpp"
#include "../include/lve_descriptors.hpp"
#include "../include/lve_swap_chain.hpp"

namespace lve
{
  class Imgui_LVE
  {
    public:
    
    Imgui_LVE(LveDevice &device, LveRenderer &render, LveWindow &window);
    ~Imgui_LVE()
    {
      ImGui_ImplVulkan_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImGui::DestroyContext();
    };

    void init();
    void draw(VkCommandBuffer commandBuffer, int index);
    VkRenderingAttachmentInfo attachment_info(VkImageView imageView, 
                             VkClearValue* clear, VkImageLayout layout);

    VkRenderingInfo rendering_info(VkExtent2D extent, VkRenderingAttachmentInfo* attachment, 
                                            VkRenderingAttachmentInfo* depth);

    private:

    LveDevice& lveDevice;
    LveRenderer& lveRenderer;
    LveWindow& lveWindow;
    VkDescriptorPool pool;
  };

}
