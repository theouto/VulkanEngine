#pragma once

#include "imconfig.h"

#include "../imgui/backends/imgui_impl_glfw.h"
#include "../imgui/backends/imgui_impl_vulkan.h"

#include <glm/glm.hpp>
#include <vector>

#include "../include/lve_game_object.hpp"
#include "../include/lve_device.hpp"
#include "../include/lve_window.hpp"
#include "../include/lve_renderer.hpp"
#include "../include/the_scene.hpp"
#include "../include/lve_descriptors.hpp"
#include "../include/lve_swap_chain.hpp"

namespace lve
{
  class Imgui_LVE
  {
    public:
    
    Imgui_LVE(LveDevice &device, LveRenderer &render, LveWindow &window, LveGameObject::Map& map, LveScene& scene);
    ~Imgui_LVE()
    {
      ImGui_ImplVulkan_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImGui::DestroyContext();
    };

    void init();
    void draw(VkCommandBuffer commandBuffer, glm::vec3 *rotationnn);
    void entityControl();
    void performance();
    void assetLoading();
    void objectLoader();
    void scene();

    private:

    glm::vec3 rot{0.f, 0.f, 0.f};
    glm::vec3 trans{0.f, 0.f, 0.f};
    glm::vec3 scale{1.f, 1.f, 1.f};

    char a[1024];
    char b[1024];
    char placeholder[1024] = {""};

    char* modelFile = a;
    char* materialFile = b;

    std::vector<bool> tabs = {true, false};
    int object = 1;
    LveScene& sceneManager;
    LveGameObject::Map& gameObjects;
    LveDevice& lveDevice;
    LveRenderer& lveRenderer;
    LveWindow& lveWindow;
    VkDescriptorPool pool;
  };

}
