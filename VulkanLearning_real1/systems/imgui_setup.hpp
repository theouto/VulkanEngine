#pragma once

#include "imconfig.h"

#include "../imgui/backends/imgui_impl_glfw.h"
#include "../imgui/backends/imgui_impl_vulkan.h"

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
    void draw(VkCommandBuffer commandBuffer, glm::vec3 *rotationnn);

    private:

    LveDevice& lveDevice;
    LveRenderer& lveRenderer;
    LveWindow& lveWindow;
    VkDescriptorPool pool;
  };

}
