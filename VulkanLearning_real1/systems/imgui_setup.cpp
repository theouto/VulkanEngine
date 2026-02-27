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

    performance();

    ImGui::Begin("lala");

    entityControl();

    ImGui::End();

    ImGui::Begin("pleasework");
    ImGui::BeginTabBar("tabs");

    //if (ImGui::TabItemButton("EntityControl"))
    //{
      if (ImGui::TabItemButton("Entities")) {tabs[0] = true; tabs[1] = false;}
      else if (ImGui::TabItemButton("Asset Loading")) {tabs[1] = true; tabs[0] = false;}
      
      if (tabs[0])// assetLoading();
      {
        for (int i = 1; i < gameObjects.size() + 1; i++)
        {
          if (ImGui::Button(gameObjects.at(i).name.c_str(), ImVec2())) object = i;
          ImGui::Spacing();
        }
      }
      else if (tabs[1]) assetLoading();
    //}
   
      //ImGui::TabItemButton("thingy");
      //assetLoading();
      //ImGui::EndTabItem();
    //}
    
    ImGui::EndTabBar();
    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
  }

  void Imgui_LVE::entityControl()
  {
    //ImGui::SliderInt("Object", &object, 1, gameObjects.size());
 
    ImGui::LabelText("\nPosition", "");
    
    ImGui::InputFloat("X", &gameObjects.at(object).transform.translation.x, 1.f, 10.f, "%.4f");
    ImGui::SameLine();
    ImGui::InputFloat("Y", &gameObjects.at(object).transform.translation.y, 1.f, 10.f);
    ImGui::SameLine();
    ImGui::InputFloat("Z", &gameObjects.at(object).transform.translation.z, 1.f, 10.f);

    ImGui::LabelText("\nRotation", "");
    ImGui::SliderFloat("X-rot", &gameObjects.at(object).transform.rotation.x, -10.f, 10.f);
    ImGui::SliderFloat("Y-rot", &gameObjects.at(object).transform.rotation.y, -10.f, 10.f);
    ImGui::SliderFloat("Z-rot", &gameObjects.at(object).transform.rotation.z, -10.f, 10.f);


    ImGui::LabelText("\nScale", "");
    ImGui::SliderFloat("X-scale", &gameObjects.at(object).transform.scale.x, -10.f, 10.f);
    ImGui::SliderFloat("Y-scale", &gameObjects.at(object).transform.scale.y, -10.f, 10.f);
    ImGui::SliderFloat("Z-scale", &gameObjects.at(object).transform.scale.z, -10.f, 10.f);
  }

  void Imgui_LVE::performance()
  {
    std::string lovely = std::to_string(ImGui::GetIO().Framerate);
    ImGui::Begin("framerate");
    ImGui::Text(lovely.c_str(), "");
    ImGui::End();
  }

  void Imgui_LVE::assetLoading()
  {
    ImGui::LabelText("\nPosition", "");
    ImGui::InputFloat("X-pos", &trans.x, -10.f, 10.f);
    ImGui::InputFloat("Y-pos", &trans.y, -10.f, 10.f);
    ImGui::InputFloat("Z-pos", &trans.z, -10.f, 10.f);


    ImGui::LabelText("\nRotation", "");
    ImGui::InputFloat("X-rot", &rot.x, -10.f, 10.f);
    ImGui::InputFloat("Y-rot", &rot.y, -10.f, 10.f);
    ImGui::InputFloat("Z-rot", &rot.z, -10.f, 10.f);


    ImGui::LabelText("\nScale", "");
    ImGui::InputFloat("X-scale", &scale.x, -10.f, 10.f);
    ImGui::InputFloat("Y-scale", &scale.y, -10.f, 10.f);
    ImGui::InputFloat("Z-scale", &scale.z, -10.f, 10.f);

    ImGui::InputText("modelFile: ", modelFile, 1024); 
    ImGui::InputText("materialFile: ", materialFile, 1024);

    if(ImGui::Button("LOAD NOW!!!", ImVec2(100.f, 20.f))) objectLoader();
}

  void Imgui_LVE::objectLoader()
  {
        std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, modelFile);
        auto object = LveGameObject::createGameObject();
        object.model = lveModel;
        object.transform.translation = trans;
        object.transform.scale = scale;
        object.transform.rotation = rot;
        object.name = modelFile;
        sceneManager.loadModel(object, *lveRenderer.globalPool, materialFile);

        scale = {1.f, 1.f, 1.f};
        rot = {0.f, 0.f, 0.f};
        trans = {0.f, 0.f, 0.f};
  }
}
