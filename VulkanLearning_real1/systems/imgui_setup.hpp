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
#include "../include/lve_swap_chain.hpp"

namespace lve
{
  class Imgui_LVE
  {
    public:
    
    Imgui_LVE(LveDevice &device, VkExtent2D happenstance);
    ~Imgui_LVE()
    {
      ImGui_ImplVulkan_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImGui::DestroyContext();
    };

    // Core functionality methods for ImGui integration
    void init(VkExtent2D happenstance);                   // Initialize ImGui context and configure display
    void initResources();                                    // Create all Vulkan resources for rendering
    void setStyle(uint32_t index);                          // Apply visual styling themes

    // Frame-by-frame rendering operations
    bool newFrame();                                         // Begin new ImGui frame and generate geometry
    void updateBuffers();                                    // Upload updated geometry to GPU buffers
    void drawFrame(vk::raii::CommandBuffer& commandBuffer); // Record rendering commands to command buffer

    // Input event handling for interactive UI elements
    void handleKey(int key, int scancode, int action, int mods); // Process keyboard input events
    bool getWantKeyCapture();                               // Query if ImGui wants keyboard focus
    void charPressed(uint32_t key);                         // Handle character input for text widgets
    private:

    LveDevice &lveDevice;

    vk::raii::Sampler sampler{nullptr};                    // Texture sampling configuration for font rendering
    VkBuffer vertexBuffer;                                    // Dynamic vertex buffer for UI geometry
    VkBuffer indexBuffer;                                     // Dynamic index buffer for UI triangle connectivity
    uint32_t vertexCount = 0;                              // Current vertex count for draw commands
    uint32_t indexCount = 0;                               // Current index count for draw commands
    VkImage fontImage;                                        // GPU texture containing ImGui font atlas
    VkImageView fontImageView;                                // Shader-accessible view of font texture

    // Vulkan pipeline infrastructure for UI rendering
    // These objects define the complete GPU processing pipeline for ImGui elements
    vk::raii::PipelineCache pipelineCache{nullptr};        // Pipeline compilation cache for faster startup
    vk::raii::PipelineLayout pipelineLayout{nullptr};      // Resource binding layout (textures, uniforms)
    vk::raii::Pipeline pipeline{nullptr};                  // Complete graphics pipeline for UI rendering
    vk::raii::DescriptorPool descriptorPool{nullptr};      // Pool for allocating descriptor sets
    vk::raii::DescriptorSetLayout descriptorSetLayout{nullptr}; // Layout defining shader resource bindings
    vk::raii::DescriptorSet descriptorSet{nullptr};        // Actual resource bindings for font texture

    // Vulkan device context and system integration
    // These references connect our UI system to the broader Vulkan application context
    vk::raii::Device* device = nullptr;                    // Primary Vulkan device for resource creation
    vk::raii::PhysicalDevice* physicalDevice = nullptr;    // GPU hardware info for capability queries
    vk::raii::Queue* graphicsQueue = nullptr;              // Command submission queue for UI rendering
    uint32_t graphicsQueueFamily = 0;                      // Queue family index for validation

    // UI state management and rendering configuration
    // These members control the visual appearance and dynamic behavior of the UI system
    ImGuiStyle vulkanStyle;                                 // Custom visual styling for Vulkan applications

    // Push constants for efficient per-frame parameter updates
    // This structure enables fast updates of transformation and styling data
    struct PushConstBlock {
        glm::vec2 scale;                                    // UI scaling factors for different screen sizes
        glm::vec2 translate;                                // Translation offset for UI positioning
    } pushConstBlock;

    // Dynamic state tracking for performance optimization
    bool needsUpdateBuffers = false;                        // Flag indicating buffer resize requirements

    // Modern Vulkan rendering configuration
    vk::PipelineRenderingCreateInfo renderingInfo{};        // Dynamic rendering setup parameters
    vk::Format colorFormat = vk::Format::eB8G8R8A8Unorm;   // Target framebuffer format

    
  };
}
