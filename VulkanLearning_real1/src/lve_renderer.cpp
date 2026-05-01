#include "../include/lve_renderer.hpp"

// std
#include <iostream>
#include <array>
#include <cassert>
#include <stdexcept>
#include <utility>
#include <vulkan/vulkan_core.h>

namespace lve {

LveRenderer::LveRenderer(LveWindow& window, LveDevice& device)
    : lveWindow{window}, lveDevice{device} 
{
  createResources();
  recreateSwapChain();
  createCommandBuffers();
}

LveRenderer::~LveRenderer() { freeCommandBuffers(); }

void LveRenderer::createResources() //I got tired of having such a dogshit renderer header file
{
    testerholyFUCK();

    globalPool = LveDescriptorPool::Builder(lveDevice)
            .setMaxSets(20)
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 65536)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 65536)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 65536)
            .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
            .build();

    descriptorPool = LveDescriptorPool::Builder(lveDevice)
            .setMaxSets(1)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1001)
            .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
            .build();

    computePool = LveDescriptorPool::Builder(lveDevice)
            .setMaxSets(2)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();

    globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)            
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
            .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
            .build();

    bindlessSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1000)
            .addBindingFlag(VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT)
            .build();

    computeSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
            .build(); //somewhat hacky...
}

void LveRenderer::generateDescriptors()
{
  globalSetLayouts.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
  computeSets.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);

  auto nerd1 = textures[0]->getDescriptorInfo();
  auto nerd2 = textures[1]->getDescriptorInfo();

  descriptorPool->allocateDescriptor(bindlessSetLayout->getDescriptorSetLayout(),
                                     bindlessLayout);

  LveDescriptorWriter(*bindlessSetLayout, *descriptorPool)
    .addImage(0, &nerd1, 0)
    .addImage(0, &nerd2, 1)
    .overwrite(bindlessLayout);

  std::cout << bindlessLayout << '\n';

  for(int i = 0; i < LveSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
  {
    auto bufferInfo = getUboInfo(i);
    auto shadowInfo = getShadowInfo();
    auto depthInfo = getDepthInfo();
    auto normalSpecInfo = getNormalInfo();

    auto renderInfo = getImageInfo(i);

    LveDescriptorWriter(*globalSetLayout, *globalPool)
      .writeBuffer(0, &bufferInfo)
      .writeImage(1, &shadowInfo)
      .writeImage(2, &depthInfo)
      .writeImage(3, &normalSpecInfo)
      .build(globalSetLayouts[i]);

    LveDescriptorWriter(*computeSetLayout, *computePool)
      .writeImage(0, &renderInfo)
      .build(computeSets[i]);
  }
}

void LveRenderer::updateDescriptors()
  {
    for(int i = 0; i < LveSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
    {
      auto bufferInfo = getUboInfo(i);
      auto shadowInfo = getShadowInfo();
      auto depthInfo = getDepthInfo();
      auto normalSpecInfo = getNormalInfo();

      auto renderInfo = getImageInfo(i);

      LveDescriptorWriter(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo) 
        .writeImage(1, &shadowInfo)
        .writeImage(2, &depthInfo)
        .writeImage(3, &normalSpecInfo)
        .overwrite(globalSetLayouts[i]);

      LveDescriptorWriter(*computeSetLayout, *computePool)
        .writeImage(0, &renderInfo)
        .overwrite(computeSets[i]);
    }
  }

void LveRenderer::recreateSwapChain() {

  if (!skip)
  {
    extent = lveWindow.getExtent();  
    while (extent.width == 0 || extent.height == 0) {
      extent = lveWindow.getExtent();
      glfwWaitEvents();
    }
    vkDeviceWaitIdle(lveDevice.device());
  }

  if (lveSwapChain == nullptr) {
    lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
  } else {
    std::shared_ptr<LveSwapChain> oldSwapChain = std::move(lveSwapChain);
    lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent, oldSwapChain);
    updateDescriptors();

    if (!oldSwapChain->compareSwapFormats(*lveSwapChain.get())) {
      throw std::runtime_error("Swap chain image(or depth) format has changed!");
    }
  }
}


void LveRenderer::createCommandBuffers() {
  commandBuffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = lveDevice.getCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo, commandBuffers.data()) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

void LveRenderer::freeCommandBuffers() {
  vkFreeCommandBuffers(
      lveDevice.device(),
      lveDevice.getCommandPool(),
      static_cast<uint32_t>(commandBuffers.size()),
      commandBuffers.data());
  commandBuffers.clear();
}

VkCommandBuffer LveRenderer::beginFrame() {
  assert(!isFrameStarted && "Can't call beginFrame while already in progress");

  auto result = lveSwapChain->acquireNextImage(&currentImageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return nullptr;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  isFrameStarted = true;

  auto commandBuffer = getCurrentCommandBuffer();
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }
  return commandBuffer;
}

void LveRenderer::endFrame() {
  assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
  auto commandBuffer = getCurrentCommandBuffer();
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }

  auto result = lveSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || lveWindow.wasWindowResized()) {
    lveWindow.resetWindowResizedFlag();
    recreateSwapChain();
  }else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  isFrameStarted = false;
  currentFrameIndex = (currentFrameIndex + 1) % LveSwapChain::MAX_FRAMES_IN_FLIGHT;
}

void LveRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
  assert(
      commandBuffer == getCurrentCommandBuffer() &&
      "Can't begin render pass on command buffer from a different frame");

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = lveSwapChain->getRenderPass();
  renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(currentImageIndex);

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
  viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void LveRenderer::beginShadowRenderPass(VkCommandBuffer commandBuffer)
{
  std::array<VkClearValue, 1> clearValues{};
  clearValues[0].depthStencil = {1.0f, 0};

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = lveSwapChain->getShadowPass();
  renderPassInfo.framebuffer = lveSwapChain->getShadowBuffer();

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = lveSwapChain->getShadowExtent();

  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = static_cast<float>(lveSwapChain->getShadowExtent().height);
  viewport.y = 0;
  viewport.width = static_cast<float>(lveSwapChain->getShadowExtent().width);
  viewport.height = static_cast<float>(lveSwapChain->getShadowExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{{0, 0}, lveSwapChain->getShadowExtent()};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void LveRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
  assert(
      commandBuffer == getCurrentCommandBuffer() &&
      "Can't end render pass on command buffer from a different frame");
  vkCmdEndRenderPass(commandBuffer);
}

void LveRenderer::beginNormalRenderPass(VkCommandBuffer commandBuffer)
  {
  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = lveSwapChain->getNormalPass();
  renderPassInfo.framebuffer = lveSwapChain->getNormalBuffer();

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
  viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
  }

  void LveRenderer::beginDepthRenderPass(VkCommandBuffer commandBuffer)
  {
    std::array<VkClearValue, 1> clearValues{};
    clearValues[0].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = lveSwapChain->getDepthPass();
    renderPassInfo.framebuffer = lveSwapChain->getDepthBuffer();

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
    viewport.y = 0;
    viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(lveSwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
  }

  void LveRenderer::testerholyFUCK()
  {
    auto nerd = LveMaterials::write_test(lveDevice);
    textures.push_back(nerd[0]);
    textures.push_back(nerd[1]);
  }

}  // namespace lve
