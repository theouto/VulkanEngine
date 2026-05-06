#include "imgui_setup.hpp"

#include <format>

#include <GLFW/glfw3.h>
#include <memory>
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
    keys = sceneManager.handler().keys();
    key = keys[0];
    files.resize(6);

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
    ImGui::BeginTabBar("other tabs");
    if (ImGui::TabItemButton("Entities")) {for (int i = 0; i < tabs.size(); i++) {tabbi[i] = i == 0 ? true : false;}}
    else if (ImGui::TabItemButton("Material Control")) {for (int i = 0; i < tabs.size(); i++) {tabbi[i] = i == 1 ? true : false;}}
    
    if (tabbi[0]) entityControl();
    else if (tabbi[1]) materialControl();
    ImGui::EndTabBar();
    ImGui::End();

    ImGui::Begin("pleasework");
    ImGui::BeginTabBar("tabs");

    if (ImGui::TabItemButton("Entities")) {for (int i = 0; i < tabs.size(); i++) {tabs[i] = i == 0 ? true : false;}}
    else if (ImGui::TabItemButton("Asset Loading")) {for (int i = 0; i < tabs.size(); i++) {tabs[i] = i == 1 ? true : false;}}
    else if (ImGui::TabItemButton("Material Selection")) {for (int i = 0; i < tabs.size(); i++) {tabs[i] = i == 2 ? true : false;}}

    if (tabs[0]) scene();
    else if (tabs[1]) assetLoading();
    else if (tabs[2]) materials();

    ImGui::EndTabBar();
    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
  }

  void Imgui_LVE::scene()
  {
    for (auto& kv : gameObjects)
    {
      if (ImGui::Button(kv.second.name.c_str(), ImVec2())) object = kv.first;
      ImGui::Spacing();
    }

    if (ImGui::Button("Save to file: ", ImVec2())) sceneManager.saveScene();
  }

  void Imgui_LVE::materials()
  {
    for (int i = 0; i < keys.size(); i++)
    { 
      if (ImGui::Button(sceneManager.handler().name(keys[i]).c_str(), ImVec2())) key = keys[i];
      ImGui::Spacing();
    }
  }

  void Imgui_LVE::entityControl()
  {
    if (ImGui::InputText("Name", placeholder, 1024)) gameObjects.at(object).name = placeholder;

    ImGui::LabelText("\nPosition", "");

    ImGui::InputFloat("X", &gameObjects.at(object).transform.translation.x, 
                      1.f, 10.f, "%.4f");
    //ImGui::SameLine();
    ImGui::InputFloat("Y", &gameObjects.at(object).transform.translation.y,
                      1.f, 10.f);
    //ImGui::SameLine();
    ImGui::InputFloat("Z", &gameObjects.at(object).transform.translation.z, 
                      1.f, 10.f);

    ImGui::LabelText("\nRotation", "");
    ImGui::SliderFloat("X-rot", &gameObjects.at(object).transform.rotation.x, -10.f, 10.f);
    ImGui::SliderFloat("Y-rot", &gameObjects.at(object).transform.rotation.y, -10.f, 10.f);
    ImGui::SliderFloat("Z-rot", &gameObjects.at(object).transform.rotation.z, -10.f, 10.f);


    ImGui::LabelText("\nScale", "");
    ImGui::SliderFloat("X-scale", &gameObjects.at(object).transform.scale.x, -10.f, 10.f);
    ImGui::SliderFloat("Y-scale", &gameObjects.at(object).transform.scale.y, -10.f, 10.f);
    ImGui::SliderFloat("Z-scale", &gameObjects.at(object).transform.scale.z, -10.f, 10.f);

    ImGui::LabelText("\nMaterial", "");
    ImGui::InputText("materialFile: ", materialFile, 1024);
    if(ImGui::Button("change material", ImVec2(100.f, 20.f))) materialChange();   

    //ImGui::InputScalar("Int", ImGuiDataType_U32, &gameObjects.at(object).textures[0], (const void *)1, (const void *)10);

    ImGui::InputInt("Shadowmap", &gameObjects.at(object).RID, 1, 1);
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

  void Imgui_LVE::materialControl()
  {
    std::vector<std::string>& files = sceneManager.handler().texFiles(key);
    ImGui::LabelText("\nMaterial textures", "");
    for (int i = 0; i < 6; i++) 
    {
        char *buf = new char[files[i].size()+1];
        strcpy(buf, files[i].c_str( ));
        this->files[i] = buf;
        ImGui::InputText(std::format("texture {}", i).c_str(), this->files[i], 1024);
        files[i] = this->files[i];
        delete[] buf;
    }


    ImGui::LabelText("\nMaterial properties", "");
    ImGui::SliderFloat("roughness", &sceneManager.handler().modi(key)[0], 0, 1);
    ImGui::SliderFloat("specular", &sceneManager.handler().modi(key)[1], 0, 1);
    ImGui::SliderFloat("ao", &sceneManager.handler().modi(key)[2], 0, 1);
    ImGui::SliderFloat("metal", &sceneManager.handler().modi(key)[3], 0, 1);

    if(ImGui::Button("Save material", ImVec2(100.f, 20.f))) sceneManager.handler().saveMaterial(key);
    if(ImGui::Button("Reload material", ImVec2(100.f, 20.f))) reloadMaterial();
  }

  void Imgui_LVE::reloadMaterial()
  {
    sceneManager.handler().reloadMaterial(key,
                                          *lveRenderer.bindlessSetLayout,
                                          *lveRenderer.descriptorPool,
                                          lveRenderer.getBindlessLayout());
  }

  void Imgui_LVE::materialChange()
  {
    sceneManager.changeMaterial(gameObjects.at(object), 
                                *lveRenderer.descriptorPool,
                                *lveRenderer.bindlessSetLayout, 
                                lveRenderer.getBindlessLayout(),
                                materialFile);

    keys = sceneManager.handler().keys();
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
        object.modelName = modelFile;
        object.matName = materialFile;
        object.type = -1;
        sceneManager.loadModel(object, *lveRenderer.globalPool, 
                                       *lveRenderer.descriptorPool, 
                                       *lveRenderer.bindlessSetLayout,
                                       lveRenderer.getBindlessLayout(),
                                        materialFile);

        keys = sceneManager.handler().keys();

        scale = {1.f, 1.f, 1.f};
        rot = {0.f, 0.f, 0.f};
        trans = {0.f, 0.f, 0.f};
  }
}
