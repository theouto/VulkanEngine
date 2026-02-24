#include "imgui_setup.hpp"

#include <format>

#include <GLFW/glfw3.h>
#include <imgui.h>
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

    ImGui_ImplGlfw_InitForVulkan(lveWindow.getGLFWwindow(), true);

    ImGui_ImplVulkan_PipelineInfo pipelineInfo{};
    pipelineInfo.RenderPass = lveRenderer.getSwapChainRenderPass();
    pipelineInfo.Subpass = 0;
    pipelineInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = lveDevice.getInstance();
    init_info.PhysicalDevice = lveDevice.getPhysicalDevice();
    init_info.Device = lveDevice.device();
    init_info.QueueFamily = lveDevice.findPhysicalQueueFamilies().graphicsFamily;
    init_info.Queue = lveDevice.graphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = lveRenderer.globalPool->getDescriptorPool();
    init_info.MinImageCount = LveSwapChain::MAX_FRAMES_IN_FLIGHT;
    init_info.ImageCount = LveSwapChain::MAX_FRAMES_IN_FLIGHT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = nullptr;
    init_info.PipelineInfoMain = pipelineInfo;

    ImGui_ImplVulkan_Init(&init_info);  

  }

  void Imgui_LVE::draw(VkCommandBuffer commandBuffer, glm::vec3 *rotationnn)
  {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame(); 
    ImGui::NewFrame();

    ImGui::Begin("Directional light");
    ImGui::SliderFloat("X", &rotationnn->x, -5.0f, 5.0f);
    ImGui::SliderFloat("Y", &rotationnn->y, 5.0f, 10.0f);
    ImGui::SliderFloat("Z", &rotationnn->z, -5.0f, 5.0f);

    ImGui::End();


    ImGui::Begin("Framerate");

    std::string lovely = std::to_string(ImGui::GetIO().Framerate);

    ImGui::PlotLines(lovely.c_str(), &ImGui::GetIO().Framerate, 30, 0, 
                     NULL, 0.f, FLT_MAX, ImVec2(0, 100.f), 0.f);

    ImGui::End();


    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
  }
}
