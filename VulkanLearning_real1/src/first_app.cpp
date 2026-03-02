#include "../include/first_app.hpp"

#include "../include/keyboard_movement_controller.hpp"
#include "../include/lve_camera.hpp"
#include "../include/lve_buffer.hpp"
#include "../include/lve_frame_info.hpp"


#include "../systems/point_light_system.hpp"
#include "../systems/simple_render_system.hpp"
#include "../systems/skybox_system.hpp"
#include "../systems/shadow_system.hpp"
#include "../systems/normal_spec.hpp"
#include "../systems/ambientocclusion_system.hpp"
#include "../systems/depth_buffer.hpp"
#include "../systems/imgui_setup.hpp"


#include <GLFW/glfw3.h>
#include <algorithm>
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <vulkan/vulkan_core.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>
#include <stdexcept>
#include <chrono>
#include <array>

namespace lve
{
	FirstApp::FirstApp()
    {
        //loadGameObjects(); deprecated
    }

	FirstApp::~FirstApp() {}


	void FirstApp::run()
	{
        std::vector<std::shared_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++)
        {
            uboBuffers[i] = std::make_unique<LveBuffer>(
                lveDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

            uboBuffers[i]->map();
        }

        lveRenderer.loadUboInfo(uboBuffers);

        std::cout << "\n\n\nhello\n\n\n";

        lveRenderer.generateDescriptors();

        std::vector<VkDescriptorSetLayout> setLayouts = {
            lveRenderer.getGlobalLayout(),
            sceneManager.mattLayout().getDescriptorSetLayout()};

        SimpleRenderSystem simpleRenderSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), setLayouts};
        PointLightSystem pointLightSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), lveRenderer.getGlobalLayout() };
        SkyboxSystem skybox{lveDevice, lveRenderer.getSwapChainRenderPass(), lveRenderer.getGlobalLayout(), *lveRenderer.globalPool};

        DirectionalLightSystem shadowSystem{lveDevice, lveRenderer.getSwapChainShadowPass(),lveRenderer.getGlobalLayout()};
        NormalSpecPass normalSpecPass{lveDevice, lveRenderer.getSwapChainNormalPass(), lveRenderer.getGlobalLayout(), setLayouts[1]};

        DepthBuffer depthBuffer{lveDevice, lveRenderer.getSwapChainDepthPass(), lveRenderer.getGlobalLayout()};
        AOSystem AOSystem{lveDevice, lveRenderer.getSwapChainRenderPass(), *lveRenderer.globalPool, lveRenderer.getGlobalLayout()};

        Imgui_LVE imgui{lveDevice, lveRenderer, lveWindow, gameObjects, sceneManager};

        LveCamera camera{};
 
        auto viewerObject = LveGameObject::createGameObject();
        viewerObject.transform.translation.z = -1.5f;
	
        
        sceneManager.load("scenes/test_scene.ths", *lveRenderer.globalPool);

	    // https://www.glfw.org/docs/3.3/input_guide.html#raw_mouse_motion <- important
        glfwSetInputMode(lveWindow.getGLFWwindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
        {
            glfwSetInputMode(lveWindow.getGLFWwindow(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }     

        glfwSetInputMode(lveWindow.getGLFWwindow(), GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
        KeyboardMovementController cameraController{};
        cameraController.mousecontrol = true;
        double mouseX = 0.f;
        double mouseY = 0.f;
	
	auto currentTime = std::chrono::high_resolution_clock::now();

    glm::vec3 rot = {1.f, 5.f, 0.f};
    std::cout << "\n\n\nAll loaded, rendering:\n\n";
    float radius = 15.f;
	while (!lveWindow.shouldClose())
	{
	    glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            //std::cout << "Framerate: " << 1 / frameTime << '\n';
            currentTime = newTime;
            
            cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject, mouseX, mouseY);
            glfwGetCursorPos(lveWindow.getGLFWwindow(), &mouseX, &mouseY);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
	

            float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.01f, 50.f);	

            glm::vec3 offset = {-radius, radius, -2.f};
		if (auto commandBuffer = lveRenderer.beginFrame())
		{		
			int frameIndex = lveRenderer.getFrameIndex();
                FrameInfo frameInfo
                {
                  frameIndex,
                  frameTime,
                  commandBuffer,
                  camera,
                  nullptr,
                  gameObjects
                };
                frameInfo.globalDescriptorSet = lveRenderer.getLayout(frameIndex);
		
                //update               
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.viewStat = camera.getviewStat();
                ubo.inverseView = camera.getInverseView();
                ubo.width = lveWindow.getExtent().width;
                ubo.height = lveWindow.getExtent().height;
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                glm::mat4 projMat = DirectionalLightSystem::lightViewProjection(
                  rot, 
                  frameInfo.camera.getPosition() + offset, 
                  radius);

                //render shadowmap
		        lveRenderer.beginShadowRenderPass(commandBuffer);
                shadowSystem.drawDepth(frameInfo, projMat, rot);
                lveRenderer.endSwapChainRenderPass(commandBuffer);

                //Depth Prepass
                lveRenderer.beginDepthRenderPass(commandBuffer);
                depthBuffer.renderGameObjects(frameInfo);
                lveRenderer.endSwapChainRenderPass(commandBuffer);

                //Normal and specular pass
                lveRenderer.beginNormalRenderPass(commandBuffer);
                normalSpecPass.drawDepth(frameInfo);
                lveRenderer.endSwapChainRenderPass(commandBuffer);
 
                //render 
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                
                //skybox
                skybox.render(frameInfo);

                //geometry pass excl. skybox
                simpleRenderSystem.renderGameObjects(frameInfo, projMat, rot);

                //Ambient Occlusion
                AOSystem.render(frameInfo);

                //renders light dots
                pointLightSystem.render(frameInfo);

                imgui.draw(commandBuffer, &rot);
                
                lveRenderer.endSwapChainRenderPass(commandBuffer);
				lveRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(lveDevice.device());
	}
}
