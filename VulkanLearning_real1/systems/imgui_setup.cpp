#include "imgui_setup.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

namespace lve 
{
  Imgui_LVE::Imgui_LVE(LveDevice &device, VkExtent2D happenstance) 
      : lveDevice{device}
  {
    init(happenstance);
  }

  void Imgui_LVE::init(VkExtent2D happenstance)
  {
    float width = happenstance.width, height = happenstance.height;

    // Initialize ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Configure ImGui
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable keyboard controls
    //io.ConfigFlags |= ImGuiConfigFlags_;      // Enable docking

    // Set display size
    io.DisplaySize = ImVec2(width, height);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    // Set up style
    vulkanStyle = ImGui::GetStyle();
    vulkanStyle.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
    vulkanStyle.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    vulkanStyle.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    vulkanStyle.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    vulkanStyle.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

    // Apply default style
    setStyle(0);
  }
 
  void Imgui_LVE::setStyle(uint32_t index)
  {
    ImGuiStyle& style = ImGui::GetStyle();

    switch (index) {
        case 0:
            // Custom Vulkan style
            style = vulkanStyle;
            break;
        case 1:
            // Classic style
            ImGui::StyleColorsClassic();
            break;
        case 2:
            // Dark style
            ImGui::StyleColorsDark();
            break;
        case 3:
            // Light style
            ImGui::StyleColorsLight();
            break;
    }
  }

  void Imgui_LVE::initResources() 
  {
    // Extract font atlas data from ImGui's internal font system
    // ImGui generates a texture atlas containing all glyphs needed for text rendering
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* fontData;                    // Raw pixel data from font atlas
    int texWidth, texHeight;                    // Dimensions of the generated font atlas
    io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

    // Calculate total memory requirements for GPU transfer
    // Each pixel contains 4 bytes (RGBA) requiring precise memory allocation
    vk::DeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);
  
        // Define image dimensions and create extent structure
    // Vulkan requires explicit specification of all image dimensions
    vk::Extent3D fontExtent{
        static_cast<uint32_t>(texWidth),        // Image width in pixels
        static_cast<uint32_t>(texHeight),       // Image height in pixels
        1                                       // Single layer (not a 3D texture or array)
    };

    // Create optimized GPU image for font texture storage
    // This image will be sampled by shaders during UI rendering
    fontImage = Image(*device, fontExtent, vk::Format::eR8G8B8A8Unorm,
                    vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
                    vk::MemoryPropertyFlagBits::eDeviceLocal);

    // Create image view for shader access
    // The image view defines how shaders interpret the raw image data
    fontImageView = VkImageView(*device, fontImage.getHandle(), vk::Format::eR8G8B8A8Unorm,
                           vk::ImageAspectFlagBits::eColor);
  }
}
