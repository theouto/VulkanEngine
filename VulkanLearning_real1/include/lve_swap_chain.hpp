#pragma once

#include "lve_device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace lve {

class LveSwapChain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
        
        LveSwapChain(LveDevice &deviceRef, VkExtent2D windowExtent);
        LveSwapChain(LveDevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr<LveSwapChain> previous);
        ~LveSwapChain();
        
        LveSwapChain(const LveSwapChain &) = delete;
        LveSwapChain &operator=(const LveSwapChain &) = delete;
        
        VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
        VkFramebuffer getShadowBuffer() {return shadowBuffer;}
        VkRenderPass getRenderPass() { return renderPass; }
        VkRenderPass getShadowPass() {return shadowPass;}
        VkImageView getImageView(int index) { return swapChainImageViews[index]; }
        VkImageView getShadowView() {return shadowDepthView;}
        size_t imageCount() { return swapChainImages.size(); }
        VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
        VkExtent2D getSwapChainExtent() { return swapChainExtent; }
        VkExtent2D getShadowExtent() {return shadowExtent;}
        uint32_t width() { return swapChainExtent.width; }
        uint32_t height() { return swapChainExtent.height; }
        
        float extentAspectRatio() 
        {
            return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }
        VkFormat findDepthFormat();
        
        VkResult acquireNextImage(uint32_t *imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

        bool compareSwapFormats(const LveSwapChain& swapChain) const
        {
            return swapChain.swapChainDepthFormat == swapChainDepthFormat && swapChain.swapChainImageFormat == swapChainImageFormat;
        }
        
    private:
        
        void init();
        void createSwapChain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();
        void createColorResources();

        void createShadowRenderPass();
        void createShadowDepthImages();
        void createShadowFrameBuffers();        
        
        // Helper functions
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
        
        VkFormat swapChainImageFormat;
        VkFormat swapChainDepthFormat;
        VkExtent2D swapChainExtent;
        
        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkRenderPass renderPass;
        
        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemorys;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        //for MSAA usage
        VkImage colorImage;
        VkDeviceMemory colorImageMemory;
        VkImageView colorImageView;
        // if it ever happens lol
 
        //shadow resources
        VkFramebuffer shadowBuffer;
        VkRenderPass shadowPass;
        VkDeviceMemory shadowMemory;
        VkImage shadowImage;
        VkImageView shadowDepthView;
        VkExtent2D shadowExtent = {512 * 2, 512 * 2};

        LveDevice &device;
        VkExtent2D windowExtent;
        
        VkSwapchainKHR swapChain;
        std::shared_ptr<LveSwapChain> oldSwapChain;
        
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        size_t currentFrame = 0;
};

}  // namespace lve
