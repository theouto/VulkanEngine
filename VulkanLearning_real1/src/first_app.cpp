#include "../include/first_app.hpp"

#include "../include/keyboard_movement_controller.hpp"
#include "../include/lve_camera.hpp"
#include "../include/lve_buffer.hpp"
#include "../include/lve_frame_info.hpp"
#include "../systems/point_light_system.hpp"
#include "../systems/simple_render_system.hpp"
#include "../systems/skybox_system.hpp"
#include "../systems/shadow_system.hpp"
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
        globalPool = LveDescriptorPool::Builder(lveDevice)
            .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT * LveGameObject::MAX_OBJECTS * 2)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT * LveGameObject::MAX_OBJECTS * 2)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LveSwapChain::MAX_FRAMES_IN_FLIGHT * LveGameObject::MAX_OBJECTS * 4)
            .build();
        loadGameObjects();
    }

	FirstApp::~FirstApp() {}


	void FirstApp::run()
	{
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        std::vector<std::unique_ptr<LveBuffer>> shadowBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
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

        auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)            
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
            .build();   

        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++)
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            auto shadowInfo = lveRenderer.getShadowInfo();

            LveDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo) 
                .writeImage(1, &shadowInfo)
                .build(globalDescriptorSets[i]);
        }

        std::vector<VkDescriptorSetLayout> setLayouts = {
            globalSetLayout->getDescriptorSetLayout(),
            matLayout->getDescriptorSetLayout()};

		SimpleRenderSystem simpleRenderSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), setLayouts};
        PointLightSystem pointLightSystem{ lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
        SkyboxSystem skybox{lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout(), *globalPool};
        DirectionalLightSystem shadowSystem{lveDevice, lveRenderer.getSwapChainShadowPass(),globalSetLayout->getDescriptorSetLayout()};
        LveCamera camera{};
 
        auto viewerObject = LveGameObject::createGameObject();
        viewerObject.transform.translation.z = -1.5f;
	

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

    std::cout << "All loaded, rendering:\n\n";

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
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.01f, 30.f);	

            glm::vec3 offset = {-10.f, 10.f, -2.f};
		if (auto commandBuffer = lveRenderer.beginFrame())
		{		
			int frameIndex = lveRenderer.getFrameIndex();
                	FrameInfo frameInfo
                	{
                 		frameIndex,
                		frameTime,
                    		commandBuffer,
                    		camera,
                    		globalDescriptorSets[frameIndex],
                    		gameObjects
                	};

		
                //update               
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.viewStat = camera.getviewStat();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();
                glm::mat4 projMat = DirectionalLightSystem::lightViewProjection(
                  {1.f, 2.f, 2.f}, frameInfo.camera.getPosition() + offset, 5.f);

                auto rotate = glm::rotate(glm::mat4(1.f), frameInfo.frameTIme, { 0.1f, -0.5f, 0.f });
                gameObjects.at(3).transform.translation = glm::vec3(rotate * glm::vec4(gameObjects.at(3).transform.translation, 1.f));


                //render shadowmap
		        lveRenderer.beginShadowRenderPass(commandBuffer);
                shadowSystem.drawDepth(frameInfo, projMat);
                lveRenderer.endSwapChainRenderPass(commandBuffer);

                //render 
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                //skybox
                skybox.render(frameInfo);

                //geometry pass excl. skybox
                simpleRenderSystem.renderGameObjects(frameInfo, projMat);

                //renders light dots
                pointLightSystem.render(frameInfo);
                
                lveRenderer.endSwapChainRenderPass(commandBuffer);
				lveRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(lveDevice.device());
	}

	void FirstApp::loadGameObjects()
	{
       std::shared_ptr<LveTextures> texSampler = std::make_shared<LveTextures>( lveDevice, 
            "textures/PavingStones115C_2K-PNG_Color.png", LveTextures::COLOR);

        std::shared_ptr<LveTextures> specular = std::make_shared<LveTextures>( lveDevice, 
            "textures/PavingStones115C_2K-PNG_Roughness.png", LveTextures::SINGLE_UNORM );
        
        std::shared_ptr<LveTextures> normals = std::make_shared<LveTextures>( lveDevice,
            "textures/PavingStones115C_2K-PNG_NormalGL.png", LveTextures::NORMAL);
        
        std::shared_ptr<LveTextures> displacement = std::make_shared<LveTextures>( lveDevice,
            "textures/PavingStones115C_2K-PNG_Displacement.png", LveTextures::SINGLE_UNORM);
        
        std::shared_ptr<LveTextures> ambOcc = std::make_shared<LveTextures>( lveDevice, 
            "textures/PavingStones115C_2K-PNG_AmbientOcclusion.png", LveTextures::SINGLE_UNORM);

        std::shared_ptr<LveTextures> metal = std::make_shared<LveTextures> (lveDevice, 
            "textures/NAM.png", LveTextures::SINGLE_UNORM);
         
        std::vector<std::shared_ptr<LveTextures>> wet_rock;
        wet_rock.push_back(texSampler);
        wet_rock.push_back(specular);
        wet_rock.push_back(normals);
        wet_rock.push_back(displacement);
        wet_rock.push_back(ambOcc);
        wet_rock.push_back(metal);

       /* 
        std::vector<std::shared_ptr<LveTextures>> planks = {std::make_unique<LveTextures>( lveDevice, "textures/Planks037A_2K-PNG_Color.png", LveTextures::COLOR ),
        std::make_unique<LveTextures>( lveDevice, "textures/Planks037A_2K-PNG_Roughness.png", LveTextures::SPECULAR ),
        std::make_unique<LveTextures>( lveDevice, "textures/Planks037A_2K-PNG_NormalGL.png", LveTextures::NORMAL ),
        std::make_unique<LveTextures>( lveDevice, "textures/Planks037A_2K-PNG_Displacement.png", LveTextures::DEPTH ),
        std::make_unique<LveTextures>( lveDevice, "textures/Planks037A_2K-PNG_AmbientOcclusion.png", LveTextures::SPECULAR)
        };
        
        

        std::vector<std::shared_ptr<LveTextures>> granite = {std::make_unique<LveTextures>( lveDevice, "textures/Granite001A_2K-PNG_Color.png", LveTextures::COLOR ),
        std::make_unique<LveTextures>( lveDevice, "textures/Granite001A_2K-PNG_Roughness.png", LveTextures::SPECULAR ),
        std::make_unique<LveTextures>( lveDevice, "textures/Granite001A_2K-PNG_NormalGL.png", LveTextures::NORMAL ),
        std::make_unique<LveTextures>( lveDevice, "textures/Granite001A_2K-PNG_Displacement.png", LveTextures::DEPTH ),
        std::make_unique<LveTextures>( lveDevice, "textures/NA.png", LveTextures::SPECULAR),
        std::make_unique<LveTextures>(lveDevice, "textures/NAM.png", LveTextures::SPECULAR)
        };
        

       
        std::vector<std::shared_ptr<LveTextures>> wet_sand = {std::make_unique<LveTextures>( lveDevice, "textures/Ground094C_4K-PNG_Color.png", LveTextures::COLOR ),
            std::make_unique<LveTextures>( lveDevice, "textures/Ground094C_4K-PNG_Roughness.png", LveTextures::SINGLE_UNORM ),
            std::make_unique<LveTextures>( lveDevice, "textures/Ground094C_4K-PNG_NormalGL.png", LveTextures::NORMAL ),
            std::make_unique<LveTextures>( lveDevice, "textures/Ground094C_4K-PNG_Displacement.png", LveTextures::SINGLE_UNORM ),
            std::make_unique<LveTextures>( lveDevice, "textures/Ground094C_4K-PNG_AmbientOcclusion.png", LveTextures::SINGLE_UNORM),
            std::make_unique<LveTextures>(lveDevice, "textures/NAM.png", LveTextures::SINGLE_UNORM)
        };
        

        
        std::vector<std::shared_ptr<LveTextures>> sMetal = {std::make_unique<LveTextures>( lveDevice, "textures/Metal051A_2K-PNG_Color.png", LveTextures::COLOR ),
            std::make_unique<LveTextures>( lveDevice, "textures/Metal051A_2K-PNG_Roughness.png", LveTextures::SINGLE_UNORM ),
            std::make_unique<LveTextures>( lveDevice, "textures/Metal051A_2K-PNG_NormalGL.png", LveTextures::NORMAL ),
            std::make_unique<LveTextures>( lveDevice, "textures/Metal051A_2K-PNG_Displacement.png", LveTextures::SINGLE_UNORM ),
            std::make_unique<LveTextures>( lveDevice, "textures/NA.png", LveTextures::SINGLE_UNORM),
            std::make_unique<LveTextures>(lveDevice, "textures/Metal051A_2K-PNG_Metalness.png", LveTextures::SINGLE_UNORM)
        };
        */

        matLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //albedo
            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //specular
            .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //normal
            .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //roughness
            .addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //AO
            .addBinding(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //metalness
            .build();
        
        std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(lveDevice, "models/smooth_vase.obj");
        auto sVase = LveGameObject::createGameObject();
        sVase.model = lveModel;
        sVase.transform.translation = { -.5f, .2f, 0.f };
        sVase.transform.scale = { 2.f, 2.f, 2.f };
        sVase.textures = wet_rock;
        gameObjects.emplace(sVase.getId(), std::move(sVase));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/flat_vase.obj");
        auto vase = LveGameObject::createGameObject();
        vase.model = lveModel;
        vase.transform.translation = { .5f, .2f, 0.f };
        vase.transform.scale = { 1.5f, 1.5f, 1.5f };
        vase.textures = wet_rock;
        gameObjects.emplace(vase.getId(), std::move(vase));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/quad.obj");
        auto quad = LveGameObject::createGameObject();
        quad.model = lveModel;
        quad.transform.translation = { 0.f, .5f, 0.f };
        quad.transform.scale = { 6.f, 1.f, 6.f };
        quad.textures = wet_rock;
        gameObjects.emplace(quad.getId(), std::move(quad));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/pleasepot.obj");
        auto pot = LveGameObject::createGameObject();
        pot.model = lveModel;
        pot.transform.translation = {0.f, -0.5f, 2.f};
        pot.transform.scale = {0.3f, -0.3f, 0.3f};
        pot.textures = wet_rock;
        gameObjects.emplace(pot.getId(), std::move(pot));

        for (auto &kv : gameObjects)
        {
          auto tex = kv.second.textures;

          auto colorInfo = tex[0]->getDescriptorInfo();
          auto specInfo = tex[1]->getDescriptorInfo();
          auto normInfo = tex[2]->getDescriptorInfo();
          auto dispInfo = tex[3]->getDescriptorInfo();
          auto ambInfo = tex[4]->getDescriptorInfo();
          auto metalInfo = tex[5]->getDescriptorInfo();

          LveDescriptorWriter(*matLayout, *globalPool)
                .writeImage(1, &colorInfo) // colour
                .writeImage(2, &specInfo) //spec 
                .writeImage(3, &normInfo) //normal
                .writeImage(4, &dispInfo) //displacement
                .writeImage(5, &ambInfo)
                .writeImage(6, &metalInfo)
                .build(kv.second.descriptorSet);
        }

        std::vector<glm::vec3> lightColors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f},
            {.0157f, 0.824f, 0.745f}    //0x04d3be
        };

        auto pointLight = LveGameObject::makePointLight(0.9f);
        pointLight.color = {1.f, 1.f, 1.f};
        pointLight.transform.translation = glm::vec3(0.7f, -0.2f, 0.7f);
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));

        for (int i = 0; i < lightColors.size(); i++) 
        {
            pointLight = LveGameObject::makePointLight(0.7f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(
                glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                { 0.f, -1.f, 0.f });
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }

        	}

}
