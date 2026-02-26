#include "imgui_setup.hpp"

#include <format>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <vulkan/vulkan_core.h>

namespace lve 
{
  Imgui_LVE::Imgui_LVE(LveDevice &device, LveRenderer &render, LveWindow &window, LveGameObject::Map& map, LveScene& scene) 
      : lveDevice{device}, lveWindow{window}, lveRenderer{render}, gameObjects{map}, sceneManager{scene}
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

    ImGui::PlotLines(lovely.c_str(), &ImGui::GetIO().Framerate, 30, 10.f, 
                     NULL, 0.f, FLT_MAX, ImVec2(0, 100.f), 0.f);

    ImGui::End();

 
    ImGui::Begin("Object control");

    ImGui::SliderInt("Object", &object, 1, gameObjects.size());
 
    ImGui::LabelText("\nPosition", "");
    ImGui::SliderFloat("X-pos", &gameObjects.at(object).transform.translation.x, -10.f, 10.f);
    ImGui::SliderFloat("Y-pos", &gameObjects.at(object).transform.translation.y, -10.f, 10.f);
    ImGui::SliderFloat("Z-pos", &gameObjects.at(object).transform.translation.z, -10.f, 10.f);


    ImGui::LabelText("\nRotation", "");
    ImGui::SliderFloat("X-rot", &gameObjects.at(object).transform.rotation.x, -10.f, 10.f);
    ImGui::SliderFloat("Y-rot", &gameObjects.at(object).transform.rotation.y, -10.f, 10.f);
    ImGui::SliderFloat("Z-rot", &gameObjects.at(object).transform.rotation.z, -10.f, 10.f);


    ImGui::LabelText("\nScale", "");
    ImGui::SliderFloat("X-scale", &gameObjects.at(object).transform.scale.x, -10.f, 10.f);
    ImGui::SliderFloat("Y-scale", &gameObjects.at(object).transform.scale.y, -10.f, 10.f);
    ImGui::SliderFloat("Z-scale", &gameObjects.at(object).transform.scale.z, -10.f, 10.f);

    if(ImGui::Button("LOAD NOW!!!", ImVec2(50.f, 20.f))) boobledybop();

    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
  }

  void Imgui_LVE::boobledybop()
  {
        //WHY THE FUCK IS THERE A NULL POSITION IN THE MAP???? WHY DO I SKIP AN ENTIRE FUCKING POSITION??? HOW?????
        sceneManager.loadModel(*lveRenderer.globalPool);
  }
}
